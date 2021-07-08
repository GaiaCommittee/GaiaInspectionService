//
// Created by vincent on 2021/7/8.
//

// You may need to build the project (run Qt uic code generator) to get "ui_TilwWindow.h" resolved

#include "TileWindow.hpp"

#include <utility>
#include <QMessageBox>
// Generated from ChartWindow.ui with QtUIC
#include "UI_TileWindow.hpp"

namespace Gaia::InspectionTile
{
    /// Constructor which will bind the inspection variables reader.
    TileWindow::TileWindow(std::unique_ptr<InspectionService::InspectionReader> &&reader,
                           std::string variable_name, unsigned int update_frequency, QWidget *parent) :
        Reader(std::move(reader)), VariableName(std::move(variable_name)), ui(new Ui::TileWindow)
    {
        ui->setupUi(this);

        if (!Reader)
        {
            QMessageBox::critical(this, "Error", "Can not connect to the inspected variable.");
            QApplication::exit(1);
        }

        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

        ui->labelName->setText(QString::fromStdString(VariableName));

        UpdateTimer = new QTimer(this);
        if (update_frequency < 0) update_frequency = 1;
        UpdateTimer->setInterval(static_cast<int>(1000 / update_frequency));

        connect(UpdateTimer, SIGNAL(timeout()), this, SLOT(OnUpdate()));

        UpdateTimer->start();
    }

    /// Destructor which will release resources.
    TileWindow::~TileWindow()
    {
        UpdateTimer->stop();
        delete ui;
    }

    /// Update displayed value.
    void TileWindow::OnUpdate()
    {
        auto result = Reader->QueryText(VariableName);
        if (!result)
        {
            ui->labelValue->setText("EMPTY");
            ui->labelValue->setStyleSheet("color: rgb(136, 138, 133);");
        }
        else
        {
            ui->labelValue->setText(QString::fromStdString(*result));
            ui->labelValue->setStyleSheet("color: rgb(114, 159, 207);");
        }
        if (*result == "true"){
            ui->labelValue->setText("TRUE");
            ui->labelValue->setStyleSheet("color: rgb(138, 226, 52);");
        }
        else if (*result == "false"){
            ui->labelValue->setText("FALSE");
            ui->labelValue->setStyleSheet("color: rgb(239, 41, 41);");
        }
    }
}
