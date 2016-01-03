/**
 * Mandelbulber v2, a 3D fractal generator
 *
 * Functions for flight animation.
 *
 * Copyright (C) 2014 Krzysztof Marczak
 *
 * This file is part of Mandelbulber.
 *
 * Mandelbulber is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Mandelbulber is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with Mandelbulber. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Krzysztof Marczak (buddhi1980@gmail.com), Sebastian Jennen
 */

#include "animation_flight.hpp"
#include "global_data.hpp"
#include "render_job.hpp"
#include "system.hpp"
#include "error_message.hpp"
#include "progress_text.hpp"
#include "headless.h"
#include <QFileDialog>
#include <QMessageBox>
#include "thumbnail_widget.h"
#include <QInputDialog>
#include "undo.h"

cFlightAnimation::cFlightAnimation(cInterface *_interface, cAnimationFrames *_frames,
		cImage *_image, QWidget *_imageWidget, cParameterContainer *_params,
		cFractalContainer *_fractal, QObject *parent) :
				QObject(parent),
				mainInterface(_interface),
				frames(_frames)
{
	if (mainInterface->mainWindow)
	{
		ui = mainInterface->mainWindow->ui;
		QApplication::connect(ui->pushButton_record_flight,
													SIGNAL(clicked()),
													this,
													SLOT(slotRecordFlight()));
		QApplication::connect(ui->pushButton_continue_recording,
													SIGNAL(clicked()),
													this,
													SLOT(slotContinueRecording()));
		QApplication::connect(ui->pushButton_render_flight,
													SIGNAL(clicked()),
													this,
													SLOT(slotRenderFlight()));
		QApplication::connect(ui->pushButton_delete_all_images,
													SIGNAL(clicked()),
													this,
													SLOT(slotDeleteAllImages()));
		QApplication::connect(ui->pushButton_show_animation,
													SIGNAL(clicked()),
													this,
													SLOT(slotShowAnimation()));
		QApplication::connect(ui->pushButton_flight_refresh_table,
													SIGNAL(clicked()),
													this,
													SLOT(slotRefreshTable()));
		QApplication::connect(ui->pushButton_flight_to_keyframe_export,
													SIGNAL(clicked()),
													this,
													SLOT(slotExportFlightToKeyframes()));

		QApplication::connect(ui->button_selectAnimFlightImageDir,
													SIGNAL(clicked()),
													this,
													SLOT(slotSelectAnimFlightImageDir()));
		QApplication::connect(mainInterface->renderedImage,
													SIGNAL(StrafeChanged(CVector2<double>)),
													this,
													SLOT(slotFlightStrafe(CVector2<double>)));
		QApplication::connect(mainInterface->renderedImage,
													SIGNAL(YawAndPitchChanged(CVector2<double>)),
													this,
													SLOT(slotFlightYawAndPitch(CVector2<double>)));
		QApplication::connect(mainInterface->renderedImage,
													SIGNAL(SpeedChanged(double)),
													this,
													SLOT(slotFlightChangeSpeed(double)));
		QApplication::connect(mainInterface->renderedImage,
													SIGNAL(RotationChanged(double)),
													this,
													SLOT(slotFlightRotation(double)));
		QApplication::connect(mainInterface->renderedImage,
													SIGNAL(Pause()),
													this,
													SLOT(slotRecordPause()));
		QApplication::connect(mainInterface->renderedImage,
													SIGNAL(ShiftModeChanged(bool)),
													this,
													SLOT(slotOrthogonalStrafe(bool)));
		QApplication::connect(ui->tableWidget_flightAnimation,
													SIGNAL(cellChanged(int, int)),
													this,
													SLOT(slotTableCellChanged(int, int)));

		QApplication::connect(ui->spinboxInt_flight_first_to_render,
													SIGNAL(valueChanged(int)),
													this,
													SLOT(slotMovedSliderFirstFrame(int)));
		QApplication::connect(ui->spinboxInt_flight_last_to_render,
													SIGNAL(valueChanged(int)),
													this,
													SLOT(slotMovedSliderLastFrame(int)));

		QApplication::connect(ui->tableWidget_flightAnimation,
													SIGNAL(cellDoubleClicked(int, int)),
													this,
													SLOT(slotCellDoubleClicked(int, int)));

		QApplication::connect(this,
													SIGNAL(QuestionMessage(const QString, const QString, QMessageBox::StandardButtons, QMessageBox::StandardButton*)),
													mainInterface->mainWindow,
													SLOT(slotQuestionMessage(const QString, const QString, QMessageBox::StandardButtons, QMessageBox::StandardButton*)));

		table = ui->tableWidget_flightAnimation;
	}
	else
	{
		ui = NULL;
		table = NULL;
	}

	QApplication::connect(this,
												SIGNAL(showErrorMessage(QString, cErrorMessage::enumMessageType, QWidget*)),
												gErrorMessage,
												SLOT(slotShowMessage(QString, cErrorMessage::enumMessageType, QWidget*)));

	image = _image;
	imageWidget = _imageWidget;
	params = _params;
	fractalParams = _fractal;
	linearSpeedSp = 0.0;
	rotationDirection = 0;
	recordPause = false;
	orthogonalStrafe = false;
}

void cFlightAnimation::slotRecordFlight()
{
	if (frames)
	{
		RecordFlight(false);
	}
	else
	{
		qCritical() << "gAnimFrames not allocated";
	}
}

void cFlightAnimation::slotContinueRecording()
{
	if (frames)
	{
		RecordFlight(true);
	}
	else
	{
		qCritical() << "gAnimFrames not allocated";
	}
}

void cFlightAnimation::slotRenderFlight()
{
	if (frames)
	{
		if (frames->GetNumberOfFrames() > 0)
		{
			RenderFlight(&gMainInterface->stopRequest);
		}
		else
		{
			emit showErrorMessage(QObject::tr("No frames to render"),
														cErrorMessage::errorMessage,
														ui->centralwidget);
		}

	}
	else
	{
		qCritical() << "gAnimFrames not allocated";
	}
}

void cFlightAnimation::RecordFlight(bool continueRecording)
{
	//get latest values of all parameters
	mainInterface->SynchronizeInterface(params, fractalParams, cInterface::read);
	gUndo.Store(params, fractalParams, frames, NULL);

	if (!continueRecording)
	{
		//confirmation dialog before start
		QMessageBox::StandardButton reply;
		reply =
				QMessageBox::question(ui->centralwidget,
															QObject::tr("Are you sure to start recording of new animation?"),
															QObject::tr("This will delete all images in the image folder.\nProceed?"),
															QMessageBox::Yes | QMessageBox::No);

		if (reply == QMessageBox::Yes)
		{
			DeleteAllFilesFromDirectory(params->Get<QString>("anim_flight_dir"), "frame_?????.*");
		}
		else
		{
			return;
		}
	}
	else
	{
		if (frames->GetNumberOfFrames() == 0)
		{
			cErrorMessage::showMessage(QObject::tr("No frames recorded before. Unable to continue."),
																 cErrorMessage::errorMessage);
			return;
		}
	}

	//check if main image is not used by other rendering process
	if (image->IsUsed())
	{
		cErrorMessage::showMessage(QObject::tr("Rendering engine is busy. Stop unfinished rendering before starting new one"),
															 cErrorMessage::errorMessage);
		return;
	}

	mainInterface->stopRequest = false;
	for(int i = 0; i < 30; i++)
	{
		if(mainInterface->stopRequest)
		{
			emit updateProgressHide();
			return;
		}
		emit updateProgressAndStatus(QObject::tr("Recordning flight path"),
																 tr("waiting %1 seconds").arg(QString::number(3.0 - 0.1 * i)),
																 0.0,
																 cProgressText::progress_ANIMATION);
		gApplication->processEvents();
		Wait(100);
	}

	if (!continueRecording)
	{
		frames->Clear();

		bool addSpeeds = params->Get<bool>("flight_add_speeds");

		//add default parameters for animation
		if (frames->GetListOfUsedParameters().size() == 0)
		{
			gAnimFrames->AddAnimatedParameter("camera", params->GetAsOneParameter("camera"));
			gAnimFrames->AddAnimatedParameter("target", params->GetAsOneParameter("target"));
			gAnimFrames->AddAnimatedParameter("camera_top", params->GetAsOneParameter("camera_top"));
			if (addSpeeds)
			{
				{
					gAnimFrames->AddAnimatedParameter("flight_movement_speed_vector",
																						params->GetAsOneParameter("flight_movement_speed_vector"));
					gAnimFrames->AddAnimatedParameter("flight_rotation_speed_vector",
																						params->GetAsOneParameter("flight_rotation_speed_vector"));
				}
			}
		}

		PrepareTable();
	}

	//setup cursor mode for renderedImage widget
	QList<QVariant> clickMode;
	clickMode.append((int) RenderedImage::clickFlightSpeedControl);
	mainInterface->renderedImage->setClickMode(clickMode);

	//setup of rendering engine
	cRenderJob *renderJob = new cRenderJob(params,
																				 fractalParams,
																				 mainInterface->mainImage,
																				 &mainInterface->stopRequest,
																				 mainInterface->renderedImage);
	connect(renderJob,
					SIGNAL(updateProgressAndStatus(const QString&, const QString&, double)),
					this,
					SIGNAL(updateProgressAndStatus(const QString&, const QString&, double)));
	connect(renderJob,
					SIGNAL(updateStatistics(cStatistics)),
					this,
					SIGNAL(updateStatistics(cStatistics)));

	cRenderingConfiguration config;
	config.DisableRefresh();
	double maxRenderTime = params->Get<double>("flight_sec_per_frame");
	;
	config.SetMaxRenderTime(maxRenderTime);

	renderJob->Init(cRenderJob::flightAnimRecord, config);
	mainInterface->stopRequest = false;

	//vector for speed and rotation control
	CVector3 cameraSpeed;
	CVector3 cameraAcceleration;
	CVector3 cameraAccelerationRotation;
	CVector3 cameraAngularSpeed;
	CVector3 cameraAngularAcceleration;
	CVector3 cameraRotation;

	int index = 0;
	if (continueRecording)
	{
		frames->GetFrameAndConsolidate(frames->GetNumberOfFrames() - 1, params, fractalParams);
		cameraSpeed = params->Get<CVector3>("flight_movement_speed_vector");
		cameraAngularSpeed = params->Get<CVector3>("flight_rotation_speed_vector");
		index = frames->GetNumberOfFrames() - 1;
	}

	CVector3 cameraPosition = params->Get<CVector3>("camera");
	CVector3 target = params->Get<CVector3>("target");
	CVector3 top = params->Get<CVector3>("camera_top");

	cCameraTarget cameraTarget(cameraPosition, target, top);

	linearSpeedSp = params->Get<double>("flight_speed");
	enumSpeedMode speedMode = (enumSpeedMode) params->Get<double>("flight_speed_control");
	double rotationSpeedSp = params->Get<double>("flight_rotation_speed") / 100.0;
	double rollSpeedSp = params->Get<double>("flight_roll_speed") / 100.0;
	double inertia = params->Get<double>("flight_inertia");

	QString framesDir = params->Get<QString>("anim_flight_dir");

	recordPause = false;

	mainInterface->progressBarAnimation->show();
	while (!mainInterface->stopRequest)
	{
		emit updateProgressAndStatus(QObject::tr("Recording flight animation"),
																 tr("Recording flight animation. Frame: ") + QString::number(index),
																 0.0,
																 cProgressText::progress_ANIMATION);

		bool wasPaused = false;
		while (recordPause)
		{
			wasPaused = true;
			emit updateProgressAndStatus(QObject::tr("Recording flight animation"),
																	 tr("Paused. Frame: ") + QString::number(index),
																	 0.0,
																	 cProgressText::progress_ANIMATION);
			gApplication->processEvents();
			if (mainInterface->stopRequest) break;
		}

		if (wasPaused)
		{
			//parameter refresh after pause
			mainInterface->SynchronizeInterface(params, fractalParams, cInterface::read);
			renderJob->UpdateParameters(params, fractalParams);
			rotationSpeedSp = params->Get<double>("flight_rotation_speed") / 100.0;
			rollSpeedSp = params->Get<double>("flight_roll_speed") / 100.0;
			inertia = params->Get<double>("flight_inertia");

			double maxRenderTime = params->Get<double>("flight_sec_per_frame");
			;
			config.SetMaxRenderTime(maxRenderTime);
			renderJob->UpdateConfig(config);

			if (mainInterface->stopRequest) break;
		}

		//speed
		double linearSpeed;
		double distanceToSurface = gMainInterface->GetDistanceForPoint(cameraPosition,
																																	 params,
																																	 fractalParams);
		if (speedMode == speedRelative)
		{
			linearSpeed = distanceToSurface * linearSpeedSp;
		}
		else
		{
			linearSpeed = linearSpeedSp;
		}

		//integrator for position
		if (strafe.Length() == 0)
		{
			cameraAcceleration = (cameraTarget.GetForwardVector() * linearSpeed - cameraSpeed)
					/ (inertia + 1.0);
		}
		else
		{
			CVector3 direction;
			if (!orthogonalStrafe) direction = cameraTarget.GetForwardVector();

			if (strafe.x != 0)
			{
				direction += cameraTarget.GetRightVector() * strafe.x;
			}
			if (strafe.y != 0)
			{
				direction += cameraTarget.GetTopVector() * strafe.y;
			}
			cameraAcceleration = (direction * linearSpeed - cameraSpeed) / (inertia + 1.0);
		}
		cameraSpeed += cameraAcceleration;
		cameraPosition += cameraSpeed;

		//rotation
		cameraAngularAcceleration.x = (yawAndPitch.x * rotationSpeedSp - cameraAngularSpeed.x)
				/ (inertia + 1.0);
		cameraAngularAcceleration.y = (yawAndPitch.y * rotationSpeedSp - cameraAngularSpeed.y)
				/ (inertia + 1.0);
		cameraAngularAcceleration.z = (rotationDirection * rollSpeedSp - cameraAngularSpeed.z)
				/ (inertia + 1.0);
		cameraAngularSpeed += cameraAngularAcceleration;

		CVector3 forwardVector = cameraTarget.GetForwardVector();
		forwardVector = forwardVector.RotateAroundVectorByAngle(cameraTarget.GetTopVector(),
																														-cameraAngularSpeed.x);
		forwardVector = forwardVector.RotateAroundVectorByAngle(cameraTarget.GetRightVector(),
																														-cameraAngularSpeed.y);

		top = cameraTarget.GetTopVector();
		top = top.RotateAroundVectorByAngle(cameraTarget.GetRightVector(), -cameraAngularSpeed.y);
		top = top.RotateAroundVectorByAngle(cameraTarget.GetForwardVector(), -cameraAngularSpeed.z);

		//update position and rotation
		target = cameraPosition + forwardVector * cameraSpeed.Length();
		cameraTarget.SetCameraTargetTop(cameraPosition, target, top);

		//update parameters
		params->Set("camera", cameraPosition);
		params->Set("target", target);
		params->Set("camera_top", top);
		params->Set("camera_rotation", cameraTarget.GetRotation() * 180.0 / M_PI);
		params->Set("camera_distance_to_target", cameraTarget.GetDistance());
		params->Set("flight_movement_speed_vector", cameraSpeed);
		params->Set("flight_rotation_speed_vector", cameraAngularSpeed);
		params->Set("frame_no", index);

		mainInterface->SynchronizeInterfaceWindow(ui->dockWidget_navigation, params, cInterface::write);
		renderJob->ChangeCameraTargetPosition(cameraTarget);

		//add new frame to container
		frames->AddFrame(*params, *fractalParams);

		//add column to table
		int newColumn = AddColumn(frames->GetFrame(frames->GetNumberOfFrames() - 1));

		//update HUD
		RenderedImage::sFlightData flightData;
		flightData.frame = frames->GetNumberOfFrames();
		flightData.camera = cameraPosition;
		flightData.speed = cameraSpeed.Length();
		flightData.speedSp = linearSpeed;
		flightData.distance = distanceToSurface;
		flightData.rotation = cameraTarget.GetRotation();
		flightData.speedVector = cameraSpeed;
		flightData.forwardVector = forwardVector;
		flightData.topVector = top;

		mainInterface->renderedImage->SetFlightData(flightData);

		//render frame
		bool result = renderJob->Execute();
		if (!result) break;

		//create thumbnail
		if (ui->checkBox_flight_show_thumbnails->isChecked())
		{
			UpdateThumbnailFromImage(newColumn);
		}

		QString filename = GetFlightFilename(index);
		SaveImage(filename, (enumImageFileType) params->Get<int>("flight_animation_image_type"), image);
		index++;
	}

	//retrieve original click mode
	QList<QVariant> item =
			ui->comboBox_mouse_click_function->itemData(ui->comboBox_mouse_click_function->currentIndex()).toList();
	gMainInterface->renderedImage->setClickMode(item);

	UpdateLimitsForFrameRange();
	ui->spinboxInt_flight_last_to_render->setValue(frames->GetNumberOfFrames());

	delete renderJob;
}

void cFlightAnimation::UpdateThumbnailFromImage(int index)
{
	table->blockSignals(true);
	QImage qimage((const uchar*) image->ConvertTo8bit(),
								image->GetWidth(),
								image->GetHeight(),
								image->GetWidth() * sizeof(sRGB8),
								QImage::Format_RGB888);
	QPixmap pixmap;
	pixmap.convertFromImage(qimage);
	QIcon icon(pixmap.scaled(QSize(100, 70),
													 Qt::KeepAspectRatioByExpanding,
													 Qt::SmoothTransformation));
	table->setItem(0, index, new QTableWidgetItem(icon, QString()));
	table->blockSignals(false);
}

void cFlightAnimation::PrepareTable()
{
	//manual delete of all cellWidgets
	//FIXME deleting of cell widgets doesn't work properly. QTableWidgets don't free memory when clear() or removeCellWidget is used
	//It calls destructors for cell widgets only when QTable widget is destroyed.
	//even if cThumbnailWidget destructors are called, there is still some copy of widget inside the table.

//	for(int i = 1; i < table->columnCount(); i++)
//	{
//		table->removeCellWidget(0, i);
//	}
//	for(int i=0; i<thumbnailWidgets.size(); i++)
//	{
//		qDebug() << thumbnailWidgets[i];
//		delete thumbnailWidgets[i];
//	}
// thumbnailWidgets.clear();

	table->setRowCount(0);
	table->setColumnCount(0);
	table->clear();
	tableRowNames.clear();
	table->verticalHeader()->setDefaultSectionSize(params->Get<int>("ui_font_size") + 6);
	CreateRowsInTable();
}

void cFlightAnimation::CreateRowsInTable()
{
	QList<cAnimationFrames::sParameterDescription> parList = frames->GetListOfUsedParameters();
	table->setIconSize(QSize(100, 70));
	table->insertRow(0);
	table->setVerticalHeaderItem(0, new QTableWidgetItem(tr("preview")));
	table->setRowHeight(0, 70);
	tableRowNames.append(tr("preview"));

	rowParameter.clear();
	rowParameter.append(-1);

	parameterRows.clear();
	for (int i = 0; i < parList.size(); ++i)
	{
		int row = AddVariableToTable(parList[i], i);
		parameterRows.append(row);
	}
}

int cFlightAnimation::AddVariableToTable(
		const cAnimationFrames::sParameterDescription &parameterDescription, int index)
{
	using namespace parameterContainer;
	enumVarType type = parameterDescription.varType;
	int row = table->rowCount();
	if (type == typeVector3)
	{
		QString varName;
		varName = parameterDescription.containerName + "_" + parameterDescription.parameterName + "_x";
		tableRowNames.append(varName);
		table->insertRow(row);
		table->setVerticalHeaderItem(row, new QTableWidgetItem(varName));
		rowParameter.append(index);

		varName = parameterDescription.containerName + "_" + parameterDescription.parameterName + "_y";
		tableRowNames.append(varName);
		table->insertRow(row + 1);
		table->setVerticalHeaderItem(row + 1, new QTableWidgetItem(varName));
		rowParameter.append(index);

		varName = parameterDescription.containerName + "_" + parameterDescription.parameterName + "_z";
		tableRowNames.append(varName);
		table->insertRow(row + 2);
		table->setVerticalHeaderItem(row + 2, new QTableWidgetItem(varName));
		rowParameter.append(index);
	}
	else if (type == typeVector4)
	{
		QString varName;
		varName = parameterDescription.containerName + "_" + parameterDescription.parameterName + "_x";
		tableRowNames.append(varName);
		table->insertRow(row);
		table->setVerticalHeaderItem(row, new QTableWidgetItem(varName));
		rowParameter.append(index);

		varName = parameterDescription.containerName + "_" + parameterDescription.parameterName + "_y";
		tableRowNames.append(varName);
		table->insertRow(row + 1);
		table->setVerticalHeaderItem(row + 1, new QTableWidgetItem(varName));
		rowParameter.append(index);

		varName = parameterDescription.containerName + "_" + parameterDescription.parameterName + "_z";
		tableRowNames.append(varName);
		table->insertRow(row + 2);
		table->setVerticalHeaderItem(row + 2, new QTableWidgetItem(varName));
		rowParameter.append(index);

		varName = parameterDescription.containerName + "_" + parameterDescription.parameterName + "_w";
		tableRowNames.append(varName);
		table->insertRow(row + 3);
		table->setVerticalHeaderItem(row + 3, new QTableWidgetItem(varName));
		rowParameter.append(index);
	}
	else if (type == typeRgb)
	{
		QString varName;
		varName = parameterDescription.containerName + "_" + parameterDescription.parameterName + "_R";
		tableRowNames.append(varName);
		table->insertRow(row);
		table->setVerticalHeaderItem(row, new QTableWidgetItem(varName));
		rowParameter.append(index);

		varName = parameterDescription.containerName + "_" + parameterDescription.parameterName + "_G";
		tableRowNames.append(varName);
		table->insertRow(row + 1);
		table->setVerticalHeaderItem(row + 1, new QTableWidgetItem(varName));
		rowParameter.append(index);

		varName = parameterDescription.containerName + "_" + parameterDescription.parameterName + "_B";
		tableRowNames.append(varName);
		table->insertRow(row + 2);
		table->setVerticalHeaderItem(row + 2, new QTableWidgetItem(varName));
		rowParameter.append(index);
	}
	else
	{
		QString varName = parameterDescription.containerName + "_" + parameterDescription.parameterName;
		tableRowNames.append(varName);
		table->insertRow(table->rowCount());
		table->setVerticalHeaderItem(table->rowCount() - 1, new QTableWidgetItem(varName));
		rowParameter.append(index);
	}
	return row;
}

int cFlightAnimation::AddColumn(const cAnimationFrames::sAnimationFrame &frame)
{
	table->blockSignals(true);
	int newColumn = table->columnCount();
	table->insertColumn(newColumn);

	QList<cAnimationFrames::sParameterDescription> parList = frames->GetListOfUsedParameters();

	using namespace parameterContainer;
	for (int i = 0; i < parList.size(); ++i)
	{
		QString parameterName = parList[i].containerName + "_" + parList[i].parameterName;
		enumVarType type = parList[i].varType;
		int row = parameterRows[i];

		if (type == typeVector3)
		{
			CVector3 val = frame.parameters.Get<CVector3>(parameterName);
			table->setItem(row, newColumn, new QTableWidgetItem(QString("%L1").arg(val.x, 0, 'g', 16)));
			table->setItem(row + 1,
										 newColumn,
										 new QTableWidgetItem(QString("%L1").arg(val.y, 0, 'g', 16)));
			table->setItem(row + 2,
										 newColumn,
										 new QTableWidgetItem(QString("%L1").arg(val.z, 0, 'g', 16)));
		}
		else if (type == typeVector4)
		{
			CVector4 val = frame.parameters.Get<CVector4>(parameterName);
			table->setItem(row, newColumn, new QTableWidgetItem(QString("%L1").arg(val.x, 0, 'g', 16)));
			table->setItem(row + 1,
										 newColumn,
										 new QTableWidgetItem(QString("%L1").arg(val.y, 0, 'g', 16)));
			table->setItem(row + 2,
										 newColumn,
										 new QTableWidgetItem(QString("%L1").arg(val.z, 0, 'g', 16)));
			table->setItem(row + 3,
										 newColumn,
										 new QTableWidgetItem(QString("%L1").arg(val.w, 0, 'g', 16)));
		}
		else if (type == typeRgb)
		{
			sRGB val = frame.parameters.Get<sRGB>(parameterName);
			table->setItem(row, newColumn, new QTableWidgetItem(QString::number(val.R)));
			table->setItem(row + 1, newColumn, new QTableWidgetItem(QString::number(val.G)));
			table->setItem(row + 2, newColumn, new QTableWidgetItem(QString::number(val.B)));
		}
		else
		{
			QString val = frame.parameters.Get<QString>(parameterName);
			table->setItem(row, newColumn, new QTableWidgetItem(val));
		}
	}
	table->blockSignals(false);
	return newColumn;
}

bool cFlightAnimation::RenderFlight(bool *stopRequest)
{
	if (image->IsUsed())
	{
		emit showErrorMessage(QObject::tr("Rendering engine is busy. Stop unfinished rendering before starting new one"),
													cErrorMessage::errorMessage);
		return false;
	}

	if (!systemData.noGui && image->IsMainImage())
	{
		mainInterface->SynchronizeInterface(params, fractalParams, cInterface::read);
		gUndo.Store(params, fractalParams, frames, NULL);
	}

	cRenderJob *renderJob = new cRenderJob(params, fractalParams, image, stopRequest, imageWidget);

	connect(renderJob,
					SIGNAL(updateProgressAndStatus(const QString&, const QString&, double)),
					this,
					SIGNAL(updateProgressAndStatus(const QString&, const QString&, double)));
	connect(renderJob,
					SIGNAL(updateStatistics(cStatistics)),
					this,
					SIGNAL(updateStatistics(cStatistics)));

	cRenderingConfiguration config;
	config.EnableNetRender();

	if (systemData.noGui)
	{
		config.DisableRefresh();
		config.DisableProgressiveRender();
		config.EnableNetRender();
	}

	renderJob->Init(cRenderJob::flightAnim, config);
	*stopRequest = false;

	QString framesDir = params->Get<QString>("anim_flight_dir");

	cProgressText progressText;
	progressText.ResetTimer();

	try
	{

		int startFrame = params->Get<int>("flight_first_to_render");
		int endFrame = params->Get<int>("flight_last_to_render");
		if (endFrame == 0) endFrame = frames->GetNumberOfFrames();

		// Check if frames have already been rendered
		for (int index = 0; index < frames->GetNumberOfFrames(); ++index)
		{
			QString filename = GetFlightFilename(index);
			cAnimationFrames::sAnimationFrame frame = frames->GetFrame(index);
			frame.alreadyRendered = QFile(filename).exists() || index < startFrame || index >= endFrame;
			frames->ModifyFrame(index, frame);
		}

		int unrenderedTotal = frames->GetUnrenderedTotal();

		if (frames->GetNumberOfFrames() > 0 && unrenderedTotal == 0)
		{
			bool deletePreviousRender = false;
			QString questionTitle = QObject::tr("Truncate Image Folder");
			QString questionText =
					QObject::tr("The animation has already been rendered completely.\n Do you want to purge the output folder?\n")
							+ QObject::tr("This will delete all images in the image folder.\nProceed?");

			if (!systemData.noGui)
			{
				QMessageBox::StandardButton reply = QMessageBox::NoButton;
				emit QuestionMessage(questionTitle,
														 questionText,
														 QMessageBox::Yes | QMessageBox::No,
														 &reply);
				while (reply == QMessageBox::NoButton)
				{
					gApplication->processEvents();
				}
				deletePreviousRender = (reply == QMessageBox::Yes);
			}
			else
			{
				deletePreviousRender = cHeadless::ConfirmMessage(questionTitle + "\n" + questionText);
			}

			if (deletePreviousRender)
			{
				DeleteAllFilesFromDirectory(params->Get<QString>("anim_flight_dir"), "frame_?????.*");
				return RenderFlight(stopRequest);
			}
			else
			{
				throw false;
			}
		}

		for (int index = 0; index < frames->GetNumberOfFrames(); ++index)
		{

			double percentDoneFrame = 0.0;
			if (unrenderedTotal > 0) percentDoneFrame = (frames->GetUnrenderedTillIndex(index) * 1.0)
					/ unrenderedTotal;
			else percentDoneFrame = 1.0;

			QString progressTxt = progressText.getText(percentDoneFrame);

			// Skip already rendered frames
			if (frames->GetFrame(index).alreadyRendered)
			{
				//int firstMissing = index;
				while (index < frames->GetNumberOfFrames() && frames->GetFrame(index).alreadyRendered)
				{
					index++;
				}
				index--;
				//qDebug() << QObject::tr("Skip already rendered frame(s) %1 - %2").arg(firstMissing).arg(index);
				continue;
			}

			emit updateProgressAndStatus(QObject::tr("Animation start"),
																	 QObject::tr("Frame %1 of %2").arg((index + 1)).arg(frames->GetNumberOfFrames())
																			 + " " + progressTxt,
																	 percentDoneFrame,
																	 cProgressText::progress_ANIMATION);

			if (*stopRequest) throw false;

			frames->GetFrameAndConsolidate(index, params, fractalParams);

			if (!systemData.noGui && image->IsMainImage())
			{
				mainInterface->SynchronizeInterface(params, fractalParams, cInterface::write);

				//show distance in statistics table
				double distance = mainInterface->GetDistanceForPoint(params->Get<CVector3>("camera"),
																														 params,
																														 fractalParams);
				ui->tableWidget_statistics->item(5, 0)->setText(QString("%L1").arg(distance));
			}

			if (gNetRender->IsServer())
			{
				gNetRender->WaitForAllClientsReady(2.0);
			}

			params->Set("frame_no", index);

			renderJob->UpdateParameters(params, fractalParams);
			int result = renderJob->Execute();
			if (!result) throw false;

			QString filename = GetFlightFilename(index);
			SaveImage(filename,
								(enumImageFileType) params->Get<int>("flight_animation_image_type"),
								image);
		}

		emit updateProgressAndStatus(QObject::tr("Animation finished"),
																 progressText.getText(1.0),
																 1.0,
																 cProgressText::progress_ANIMATION);
		emit updateProgressHide();
	} catch (bool ex)
	{
		emit updateProgressAndStatus(QObject::tr("Rendering terminated"),
																 progressText.getText(1.0),
																 cProgressText::progress_ANIMATION);
		emit updateProgressHide();
		delete renderJob;
		return false;
	}

	delete renderJob;
	return true;
}

void cFlightAnimation::RefreshTable()
{
	PrepareTable();
	gApplication->processEvents();

	int noOfFrames = frames->GetNumberOfFrames();

	UpdateLimitsForFrameRange(); //it is needed to do it also here, because limits must be set just after loading of settings

	mainInterface->SynchronizeInterfaceWindow(ui->tab_flight_animation, params, cInterface::read);
	cParameterContainer tempPar = *params;
	cFractalContainer tempFract = *fractalParams;

	for (int i = 0; i < noOfFrames; i++)
	{
		int newColumn = AddColumn(frames->GetFrame(i));

		if (ui->checkBox_flight_show_thumbnails->isChecked())
		{
			cThumbnailWidget *thumbWidget = new cThumbnailWidget(100, 70, table);
			thumbWidget->UseOneCPUCore(true);
			frames->GetFrameAndConsolidate(i, &tempPar, &tempFract);
			thumbWidget->AssignParameters(tempPar, tempFract);
			table->setCellWidget(0, newColumn, thumbWidget);
		}
		if (i % 100 == 0)
		{
			emit updateProgressAndStatus(QObject::tr("Refreshing animation"),
																	 tr("Refreshing animation frames"),
																	 (double) i / noOfFrames,
																	 cProgressText::progress_ANIMATION);
			gApplication->processEvents();
		}
	}
	UpdateLimitsForFrameRange();
	emit updateProgressHide();
}

QString cFlightAnimation::GetParameterName(int rowNumber)
{
	int parameterNumber = rowParameter[rowNumber];

	QString fullParameterName;
	QList<cAnimationFrames::sParameterDescription> list = frames->GetListOfUsedParameters();
	if (parameterNumber >= 0)
	{
		fullParameterName = list[parameterNumber].containerName + "_"
				+ list[parameterNumber].parameterName;
	}
	else
	{
		qCritical() << "cFlightAnimation::GetParameterNumber(int rowNumber): row not found";
	}
	return fullParameterName;
}

void cFlightAnimation::RenderFrame(int index)
{
	mainInterface->SynchronizeInterface(params, fractalParams, cInterface::read);
	frames->GetFrameAndConsolidate(index, params, fractalParams);
	mainInterface->SynchronizeInterface(params, fractalParams, cInterface::write);

	mainInterface->StartRender();
}

void cFlightAnimation::DeleteFramesFrom(int index)
{
	gUndo.Store(params, fractalParams, frames, NULL);
	for (int i = frames->GetNumberOfFrames() - 1; i >= index; i--)
		table->removeColumn(index);
	frames->DeleteFrames(index, frames->GetNumberOfFrames() - 1);
	UpdateLimitsForFrameRange();
}

void cFlightAnimation::DeleteFramesTo(int index)
{
	gUndo.Store(params, fractalParams, frames, NULL);
	for (int i = 0; i <= index; i++)
		table->removeColumn(0);
	frames->DeleteFrames(0, index);
	UpdateLimitsForFrameRange();
}

void cFlightAnimation::slotFlightStrafe(CVector2<double> _strafe)
{
	strafe = _strafe;
}

void cFlightAnimation::slotFlightYawAndPitch(CVector2<double> _yawAndPitch)
{
	yawAndPitch = _yawAndPitch;
}

void cFlightAnimation::slotFlightChangeSpeed(double amount)
{
	mainInterface->SynchronizeInterfaceWindow(ui->scrollAreaWidgetContents_flightAnimationParameters,
																						params,
																						cInterface::read);
	linearSpeedSp = params->Get<double>("flight_speed") * amount;
	params->Set("flight_speed", linearSpeedSp);
	mainInterface->SynchronizeInterfaceWindow(ui->scrollAreaWidgetContents_flightAnimationParameters,
																						params,
																						cInterface::write);
}

void cFlightAnimation::slotFlightRotation(double direction)
{
	rotationDirection = direction;
}

void cFlightAnimation::slotOrthogonalStrafe(bool _orthogonalStrafe)
{
	orthogonalStrafe = _orthogonalStrafe;
}

void cFlightAnimation::slotSelectAnimFlightImageDir()
{
	QFileDialog* dialog = new QFileDialog();
	dialog->setFileMode(QFileDialog::DirectoryOnly);
	dialog->setNameFilter(QObject::tr("Animation Image Folder"));
	dialog->setDirectory(QDir::toNativeSeparators(params->Get<QString>("anim_flight_dir")));
	dialog->setAcceptMode(QFileDialog::AcceptOpen);
	dialog->setWindowTitle(QObject::tr("Choose Animation Image Folder"));
	dialog->setOption(QFileDialog::ShowDirsOnly);
	QStringList filenames;

	if (dialog->exec())
	{
		filenames = dialog->selectedFiles();
		QString filename = QDir::toNativeSeparators(filenames.first() + QDir::separator());
		ui->text_anim_flight_dir->setText(filename);
		params->Set("anim_flight_dir", filename);
	}
}

void cFlightAnimation::slotTableCellChanged(int row, int column)
{
	if (row > 0)
	{
		table->blockSignals(true);
		QTableWidgetItem *cell = table->item(row, column);
		QString cellText = cell->text();

		cAnimationFrames::sAnimationFrame frame = frames->GetFrame(column);

		QString parameterName = GetParameterName(row);
		int parameterFirstRow = parameterRows[rowParameter[row]];
		int vectIndex = row - parameterFirstRow;

		using namespace parameterContainer;
		enumVarType type = frame.parameters.GetVarType(parameterName);

		if (type == typeVector3)
		{
			CVector3 vect = frame.parameters.Get<CVector3>(parameterName);
			if (vectIndex == 0) vect.x = systemData.locale.toDouble(cellText);
			if (vectIndex == 1) vect.y = systemData.locale.toDouble(cellText);
			if (vectIndex == 2) vect.z = systemData.locale.toDouble(cellText);
			frame.parameters.Set(parameterName, vect);
		}
		else if (type == typeVector4)
		{
			CVector4 vect = frame.parameters.Get<CVector4>(parameterName);
			if (vectIndex == 0) vect.x = systemData.locale.toDouble(cellText);
			if (vectIndex == 1) vect.y = systemData.locale.toDouble(cellText);
			if (vectIndex == 2) vect.z = systemData.locale.toDouble(cellText);
			if (vectIndex == 3) vect.w = systemData.locale.toDouble(cellText);
			frame.parameters.Set(parameterName, vect);
		}
		else if (type == typeRgb)
		{
			sRGB col = frame.parameters.Get<sRGB>(parameterName);
			if (vectIndex == 0) col.R = cellText.toInt();
			if (vectIndex == 1) col.G = cellText.toInt();
			if (vectIndex == 2) col.B = cellText.toInt();
			frame.parameters.Set(parameterName, col);
		}
		else
		{
			frame.parameters.Set(parameterName, cellText);
		}

		frames->ModifyFrame(column, frame);

		//update thumbnail
		if (ui->checkBox_flight_show_thumbnails->isChecked())
		{
			cParameterContainer tempPar = *params;
			cFractalContainer tempFract = *fractalParams;
			frames->GetFrameAndConsolidate(column, &tempPar, &tempFract);
			cThumbnailWidget *thumbWidget = (cThumbnailWidget*) table->cellWidget(0, column);

			if (!thumbWidget)
			{
				cThumbnailWidget *thumbWidget = new cThumbnailWidget(100, 70, table);
				thumbWidget->UseOneCPUCore(true);
				thumbWidget->AssignParameters(tempPar, tempFract);
				table->setCellWidget(0, column, thumbWidget);
			}
			else
			{
				thumbWidget->AssignParameters(tempPar, tempFract);
			}
		}

		table->blockSignals(false);
	}
}

void cFlightAnimation::slotDeleteAllImages()
{
	mainInterface->SynchronizeInterfaceWindow(ui->scrollAreaWidgetContents_flightAnimationParameters,
																						params,
																						cInterface::read);

	QMessageBox::StandardButton reply;
	reply =
			QMessageBox::question(ui->centralwidget,
														QObject::tr("Truncate Image Folder"),
														QObject::tr("This will delete all images in the image folder.\nProceed?"),
														QMessageBox::Yes | QMessageBox::No);

	if (reply == QMessageBox::Yes)
	{
		DeleteAllFilesFromDirectory(params->Get<QString>("anim_flight_dir"), "frame_?????.*");
	}
}

void cFlightAnimation::slotShowAnimation()
{
	WriteLog("Prepare PlayerWidget class");
	mainInterface->SynchronizeInterfaceWindow(ui->scrollAreaWidgetContents_keyframeAnimationParameters,
																						params,
																						cInterface::read);
	mainInterface->imageSequencePlayer = new PlayerWidget();
	mainInterface->imageSequencePlayer->SetFilePath(params->Get<QString>("anim_flight_dir"));
	mainInterface->imageSequencePlayer->show();
}

void cFlightAnimation::slotRecordPause()
{
	recordPause = !recordPause;
	//qDebug() << recordPause;
}

void cFlightAnimation::InterpolateForward(int row, int column)
{
	gUndo.Store(params, fractalParams, frames, NULL);

	QTableWidgetItem *cell = ui->tableWidget_flightAnimation->item(row, column);
	QString cellText = cell->text();

	QString parameterName = GetParameterName(row);

	cAnimationFrames::sAnimationFrame frame = frames->GetFrame(column);

	using namespace parameterContainer;
	enumVarType type = frame.parameters.GetVarType(parameterName);

	bool valueIsInteger = false;
	bool valueIsDouble = false;
	bool valueIsText = false;
	int valueInteger = 0;
	double valueDouble = 0.0;
	QString valueText;

	bool ok;
	int lastFrame = QInputDialog::getInt(mainInterface->mainWindow,
																			 "Parameter interpolation",
																			 "Enter last frame number",
																			 column + 1,
																			 column + 2,
																			 frames->GetNumberOfFrames(),
																			 1,
																			 &ok);
	if (!ok) return;

	int numberOfFrames = (lastFrame - column - 1);

	switch (type)
	{
		case typeBool:
		case typeInt:
		case typeRgb:
		{
			valueIsInteger = true;
			valueInteger = cellText.toInt();
			//qDebug() << valueInteger;
			break;
		}
		case typeDouble:
		case typeVector3:
		case typeVector4:
		{
			valueIsDouble = true;
			valueDouble = systemData.locale.toDouble(cellText);
			//qDebug() << valueDouble;
			break;
		}
		default:
		{
			valueIsText = true;
			valueText = cellText;
			break;
		}
	}

	int finalInteger = 0;
	double finalDouble = 0.0;
	double integerStep = 0.0;
	double doubleStep = 0.0;

	if (valueIsInteger)
	{
		finalInteger = QInputDialog::getInt(mainInterface->mainWindow,
																				"Parameter interpolation",
																				"Enter value for last frame",
																				valueInteger,
																				0,
																				2147483647,
																				1,
																				&ok);
		integerStep = (double) (finalInteger - valueInteger) / numberOfFrames;
	}
	else if (valueIsDouble)
	{
		finalDouble = systemData.locale.toDouble(QInputDialog::getText(mainInterface->mainWindow,
																																	 "Parameter interpolation",
																																	 "Enter value for last frame",
																																	 QLineEdit::Normal,
																																	 QString("%L1").arg(valueDouble,
																																											0,
																																											'g',
																																											16),
																																	 &ok));
		doubleStep = (finalDouble - valueDouble) / numberOfFrames;
	}

	if (!ok) return;

	for (int i = column; i < lastFrame; i++)
	{
		QString newCellText;
		if (valueIsInteger)
		{
			int newValue = integerStep * (i - column) + valueInteger;
			newCellText = QString::number(newValue);
		}
		else if (valueIsDouble)
		{
			double newValue = doubleStep * (i - column) + valueDouble;
			newCellText = QString("%L1").arg(newValue, 0, 'g', 16);
		}
		else if (valueIsText)
		{
			newCellText = valueText;
		}
		QTableWidgetItem *newCell = ui->tableWidget_flightAnimation->item(row, i);
		newCell->setText(newCellText);
	}
}

void cFlightAnimation::slotRefreshTable()
{
	RefreshTable();
}

QString cFlightAnimation::GetFlightFilename(int index)
{
	QString filename = params->Get<QString>("anim_flight_dir") + "frame_"
			+ QString("%1").arg(index, 5, 10, QChar('0'));
	switch ((enumImageFileType) params->Get<double>("flight_animation_image_type"))
	{
		case IMAGE_FILE_TYPE_JPG:
			filename += QString(".jpg");
			break;
		case IMAGE_FILE_TYPE_PNG:
			filename += QString(".png");
			break;
		case IMAGE_FILE_TYPE_EXR:
			filename += QString(".exr");
			break;
	}
	return filename;
}

void cFlightAnimation::slotExportFlightToKeyframes()
{
	mainInterface->SynchronizeInterface(params, fractalParams, cInterface::read);
	gUndo.Store(params, fractalParams, gAnimFrames, gKeyframes);

	if (gKeyframes->GetFrames().size() > 0)
	{
		QMessageBox::StandardButton reply;
		reply =
				QMessageBox::question(ui->centralwidget,
															QObject::tr("Export flight to keyframes"),
															QObject::tr("There are already captured keyframes present.\nDiscard current keyframes?"),
															QMessageBox::Yes | QMessageBox::No);

		if (reply == QMessageBox::No) return;
	}

	gKeyframes->ClearAll();
	gKeyframes->ClearMorphCache();
	gKeyframes->SetListOfParametersAndClear(gAnimFrames->GetListOfParameters());

	int step = params->Get<int>("frames_per_keyframe");

	for (int i = 0; i < frames->GetNumberOfFrames(); i += step)
	{
		gKeyframes->AddFrame(frames->GetFrame(i));
	}

	ui->tabWidgetFlightKeyframe->setCurrentIndex(1);
	ui->pushButton_refresh_keyframe_table->animateClick();
}

void cFlightAnimation::UpdateLimitsForFrameRange(void)
{
	int noOfFrames = frames->GetNumberOfFrames();

	ui->spinboxInt_flight_first_to_render->setMaximum(noOfFrames);
	ui->sliderInt_flight_first_to_render->setMaximum(noOfFrames);

	ui->spinboxInt_flight_last_to_render->setMaximum(noOfFrames);
	ui->sliderInt_flight_last_to_render->setMaximum(noOfFrames);

	mainInterface->SynchronizeInterfaceWindow(ui->tab_flight_animation, gPar, cInterface::write);
}

void cFlightAnimation::slotMovedSliderFirstFrame(int value)
{
	if (value > ui->spinboxInt_flight_last_to_render->value()) ui->spinboxInt_flight_last_to_render->setValue(value);
}
void cFlightAnimation::slotMovedSliderLastFrame(int value)
{
	if (value < ui->spinboxInt_flight_first_to_render->value()) ui->spinboxInt_flight_first_to_render->setValue(value);
}

void cFlightAnimation::slotCellDoubleClicked(int row, int column)
{
	if (row == 0)
	{
		RenderFrame(column);
	}
}
