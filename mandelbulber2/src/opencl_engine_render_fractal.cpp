/*
 * opencl_engine_render_fractal.cpp
 *
 *  Created on: 6 maj 2017
 *      Author: krzysztof
 */

#include "opencl_engine_render_fractal.h"
#include "files.h"
#include "fractparams.hpp"
#include "opencl_hardware.h"
#include "fractal_list.hpp"
#include "cimage.hpp"

cOpenClEngineRenderFractal::cOpenClEngineRenderFractal(cOpenClHardware *_hardware)
		: cOpenClEngine(_hardware)
{
#ifdef USE_OPENCL
	constantInBuffer = nullptr;
	inCLConstBuffer = nullptr;

	inBuffer = nullptr;
	inCLBuffer = nullptr;

	rgbbuff = nullptr;
	outCL = nullptr;
#endif
}

cOpenClEngineRenderFractal::~cOpenClEngineRenderFractal()
{
#ifdef USE_OPENCL
	if (constantInBuffer) delete constantInBuffer;
	if (inCLConstBuffer) delete inCLConstBuffer;
	if (inBuffer) delete inBuffer;
	if (inCLBuffer) delete inCLBuffer;
	if (rgbbuff) delete rgbbuff;
	if (outCL) delete outCL;
#endif
}

#ifdef USE_OPENCL
void cOpenClEngineRenderFractal::LoadSourcesAndCompile(const cParameterContainer *params)
{
	QString progPathHeader("#define INCLUDE_PATH_CL_DATA \"");

// pushing path to mandelbulber_cl_data.h
// using correct slashes is important, because OpenCl compiler would fail
#ifdef WIN32
	progPathHeader += systemData.sharedDir + "opencl\\mandelbulber_cl_data.h\"";
#else
	progPathHeader += systemData.sharedDir + "opencl/mandelbulber_cl_data.h\"";
#endif
	QByteArray progPathHeaderUft8 = progPathHeader.toUtf8();
	qDebug() << progPathHeader;

	programsLoaded = false;
	readyForRendering = false;

	QByteArray progEngine;
	try
	{
		progEngine = LoadUtf8TextFromFile(systemData.sharedDir + "opencl" + QDir::separator()
																			+ "engines" + QDir::separator() + "test_engine.cl");
		if (progEngine.isEmpty()) throw QString("Can't load main program");

		/*
		// Test loading of all fractal kernels
		for (int f = 0; f < fractalList.size(); f++)
		{
			progEngine.append(LoadUtf8TextFromFile(fractalList[f].getOpenCLFilename()));
		}
		*/

		//.... here will be loading of more programs
	}
	catch (const QString &ex)
	{
		qCritical() << "OpenCl program error: " << ex;
		return;
	}

	// collecting all parts of program
	cl::Program::Sources sources;
	sources.push_back(
		std::make_pair(progPathHeaderUft8.constData(), size_t(progPathHeader.length())));
	sources.push_back(std::make_pair(progEngine.constData(), size_t(progEngine.length())));

	// creating cl::Program
	cl_int err;

	if (program) delete program;
	program = new cl::Program(*hardware->getContext(), sources, &err);
	if (checkErr(err, "cl::Program()"))
	{
		// building OpenCl kernel

		QString errorString;
		if (Build(program, &errorString))
		{
			programsLoaded = true;
		}
	}
}

void cOpenClEngineRenderFractal::SetParameters(const cParameterContainer *params)
{
	if (constantInBuffer) delete constantInBuffer;
	constantInBuffer = new sClInConstants;

	// TODO Write function to copy parameters from cParamRender to sClParamRender
	// Would be good to write php script for it
	cParamRender *paramRender = new cParamRender(params);

	constantInBuffer->params.N = paramRender->N;
	constantInBuffer->params.imageWidth = paramRender->imageWidth;
	constantInBuffer->params.imageHeight = paramRender->imageHeight;
	delete paramRender;
}

bool cOpenClEngineRenderFractal::PreAllocateBuffers(const cParameterContainer *params)
{
	cl_int err;

	if (inCLConstBuffer) delete inCLConstBuffer;
	inCLConstBuffer = new cl::Buffer(*hardware->getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(sClInConstants), constantInBuffer, &err);
	if (!checkErr(err,
				"cl::Buffer(*hardware->getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, "
				"sizeof(sClInConstants), constantInBuffer, &err)"))
		return false;

	// this buffer will be used for color palettes, lights, etc...
	if (inBuffer) delete inBuffer;
	inBuffer = new sClInBuff;
	if (inCLBuffer) delete inCLBuffer;
	inCLBuffer = new cl::Buffer(*hardware->getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(sClInBuff), inBuffer, &err);
	if (!checkErr(err,
				"Buffer::Buffer(*hardware->getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, "
				"sizeof(sClInBuff), inBuffer, &err)"))
		return false;

	size_t buffSize = optimalJob.stepSize * sizeof(sClPixel);
	if (rgbbuff) delete rgbbuff;
	rgbbuff = new sClPixel[buffSize];

	if (outCL) delete outCL;
	outCL = new cl::Buffer(
		*hardware->getContext(), CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, buffSize, rgbbuff, &err);
	if (!checkErr(err, "*context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, buffSize, rgbbuff, &err"))
		return false;

	return true;
}

bool cOpenClEngineRenderFractal::Render(cImage *image)
{
	int width = image->GetWidth();
	int height = image->GetHeight();

	size_t sizeOfPixel = sizeof(sClPixel);

	for (int pixelIndex = 0; pixelIndex < width * height; pixelIndex += optimalJob.stepSize)
	{
		cl_int err;

		size_t buffSize = optimalJob.stepSize * sizeof(sClPixel);

		// assign parameters to kernel
		err = kernel->setArg(0, *outCL); // output image
		if (!checkErr(err, "kernel->setArg(0, *outCL)")) return false;
		err = kernel->setArg(1, *inCLBuffer); // input data in global memory
		if (!checkErr(err, "kernel->setArg(1, *inCLBuffer)")) return false;
		err = kernel->setArg(2, *inCLConstBuffer); // input data in constant memory (faster than global)
		if (!checkErr(err, "kernel->setArg(2, *inCLConstBuffer)")) return false;
		err = kernel->setArg(3, pixelIndex); // pixel offset
		if (!checkErr(err, "kernel->setArg(3, pixelIndex)")) return false;

		// writing data to queue
		err = queue->enqueueWriteBuffer(*inCLBuffer, CL_TRUE, 0, sizeof(sClInBuff), inBuffer);
		size_t usedGPUdMem = sizeOfPixel * optimalJob.stepSize;
		qDebug() << "Used GPU mem (KB): " << usedGPUdMem / 1024;
		if (!checkErr(err, "ComamndQueue::enqueueWriteBuffer(inCLBuffer)")) return false;

		err = queue->finish();
		if (!checkErr(err, "ComamndQueue::finish() - inCLBuffer")) return false;

		err = queue->enqueueWriteBuffer(
			*inCLConstBuffer, CL_TRUE, 0, sizeof(sClInConstants), constantInBuffer);
		if (!checkErr(err, "ComamndQueue::enqueueWriteBuffer(inCLConstBuffer)")) return false;

		err = queue->finish();
		if (!checkErr(err, "ComamndQueue::finish() - inCLConstBuffer")) return false;

		// processing queue
		err = queue->enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange(optimalJob.stepSize),
			cl::NDRange(optimalJob.workGroupSize));
		if (!checkErr(err, "ComamndQueue::enqueueNDRangeKernel()")) return false;

		err = queue->enqueueReadBuffer(*outCL, CL_TRUE, 0, buffSize, rgbbuff);
		if (!checkErr(err, "ComamndQueue::enqueueReadBuffer()")) return false;
		err = queue->finish();
		if (!checkErr(err, "ComamndQueue::finish() - ReadBuffer")) return false;

		for (unsigned int i = 0; i < optimalJob.stepSize; i++)
		{
			unsigned int a = pixelIndex + i;
			sClPixel pixelCl = rgbbuff[i];
			sRGBFloat pixel = {pixelCl.R, pixelCl.G, pixelCl.B};
			sRGB8 color = {pixelCl.colR, pixelCl.colG, pixelCl.colB};
			unsigned short opacity = pixelCl.opacity;
			unsigned short alpha = pixelCl.alpha;
			int x = a % width;
			int y = a / width;

			image->PutPixelImage(x, y, pixel);
			image->PutPixelZBuffer(x, y, rgbbuff[i].zBuffer);
			image->PutPixelColour(x, y, color);
			image->PutPixelOpacity(x, y, opacity);
			image->PutPixelAlpha(x, y, alpha);
		}
	}

	qDebug() << "GPU jobs finished";

	// refresh image at end
	image->NullPostEffect();

	WriteLog("image->CompileImage()", 2);
	image->CompileImage();

	if (image->IsPreview())
	{
		WriteLog("image->ConvertTo8bit()", 2);
		image->ConvertTo8bit();
		WriteLog("image->UpdatePreview()", 2);
		image->UpdatePreview();
		WriteLog("image->GetImageWidget()->update()", 2);
		image->GetImageWidget()->update();
	}

	return true;
}

#endif