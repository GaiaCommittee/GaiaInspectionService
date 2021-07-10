//
// Created by vincent on 2021/7/8.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ChartWindow.h" resolved

#include "ChartWindow.hpp"
// Generated from ChartWindow.ui with QtUIC
#include "ui_ChartWindow.h"

#include <QMessageBox>
#include <algorithm>

namespace Gaia::InspectionChart
{
    /// Connect to the given reader and build the window.
    ChartWindow::ChartWindow(std::unique_ptr<InspectionService::InspectionReader>&& reader, QWidget *parent) :
        Reader(std::move(reader)), QMainWindow(parent), ui(new Ui::ChartWindow)
    {
        ui->setupUi(this);

        if (!Reader)
        {
            QMessageBox::critical(this, "Error", "Can not connect to the inspected variable.");
            QApplication::exit(1);
        }

        if (Reader->GetUnitName().empty())
        {
            Reader->BindUnit(QInputDialog::getText(this, "Gaia Inspection - Chart",
                                                   "Input Unit Name").toStdString());
        }

        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

        auto variable_names = Reader->QueryVariables();
        for (const auto& variable_name : variable_names)
        {
            ui->nameCombo->addItem(QString::fromStdString(variable_name));
        }

        UpdateTimer = new QTimer(this);
        UpdateTimer->setInterval(1000);

        ChartModel = new QtCharts::QChart();
        ChartView = new QtCharts::QChartView(this);
        ChartView->setChart(ChartModel);
        ui->viewLayout->addWidget(ChartView);

        AxisX = new QValueAxis(this);
        AxisX->setRange(0, 20);
        AxisX->setTitleText("Frame");
        AxisX->setTickInterval(1.0);
        ChartModel->addAxis(AxisX, Qt::AlignmentFlag::AlignBottom);

        AxisY = new QValueAxis(this);
        AxisY->setRange(-1000, 1000);
        AxisY->setTitleText("Value");
        AxisY->setTickInterval(1.0);
        ChartModel->addAxis(AxisY, Qt::AlignmentFlag::AlignLeft);

        ChartData = new QtCharts::QLineSeries(this);
        ChartData->setName("Value");
        ChartModel->addSeries(ChartData);
        ChartData->attachAxis(AxisX);
        ChartData->attachAxis(AxisY);

        connect(ui->frequencySpin, SIGNAL(valueChanged(int)),
                this, SLOT(OnFrequencyChanged(int)));
        connect(ui->nameCombo, SIGNAL(currentTextChanged(const QString &)),
                this, SLOT(OnVariableChanged(QString)));
        connect(UpdateTimer, SIGNAL(timeout()), this, SLOT(OnUpdate()));

        UpdateTimer->start();

        if (!ui->nameCombo->currentText().isEmpty())
        {
            VariableName = ui->nameCombo->currentText().toStdString();
        }
    }

    /// Release resources.
    ChartWindow::~ChartWindow()
    {
        delete ChartModel;
        delete ui;
    }

    /// Change the interval time of timer.
    void ChartWindow::OnFrequencyChanged(int value)
    {
        UpdateTimer->stop();
        UpdateTimer->setInterval(1000 / value);
        UpdateTimer->start();
    }

    /// Update and add value into the chart.
    void ChartWindow::OnUpdate()
    {
        auto value_text = Reader->QueryText(VariableName);
        ui->labelValue->setText(QString::fromStdString(value_text.has_value() ? *value_text : "(Empty)"));
        if (!value_text.has_value()) return;

        double current_value;
        try
        {
            current_value = std::stod(*value_text);
            ChartData->append(static_cast<qreal>(NextRecordIndex), current_value);

            ++NextRecordIndex;

            unsigned int max_records_columns = ui->centralwidget->size().width() / 10;
            unsigned int current_records = ChartData->count();
            if (current_records > max_records_columns)
            {
                ChartData->removePoints(0,static_cast<int>(current_records - max_records_columns));
            }

            const auto& data = ChartData->points();
            auto [min_iterator, max_iterator] = std::minmax_element(data.begin(), data.end(), [](const QPointF& v1, const QPointF& v2)
            {
                return v1.y() < v2.y();
            });
            auto min_value = min_iterator->y();
            auto max_value = max_iterator->y();
            double lower_bound = min_value;
            double upper_bound = max_value;
            auto difference = max_value - min_value;
            if (min_value * (min_value - (difference / 10)) > 0)
            {
                lower_bound -= difference / 10 + 1.0;
            }
            upper_bound += difference / 10 + 1.0;
            if (upper_bound < 1.0) upper_bound = 1.0;
            if (lower_bound < 0.0 && lower_bound * min_value < 0) lower_bound = 0.0f;
            AxisY->setRange(lower_bound, upper_bound);
            AxisX->setRange(
                    NextRecordIndex <= max_records_columns ? 0 : static_cast<qreal>(NextRecordIndex - current_records - 1),
                    static_cast<qreal>(NextRecordIndex - current_records - 1 + max_records_columns));
            AxisX->setTickCount(static_cast<int>(max_records_columns / 10));
        }
        catch(std::invalid_argument& error)
        {
        }
    }

    /// Change the bound variable name.
    void ChartWindow::OnVariableChanged(const QString& name)
    {
        VariableName = name.toStdString();
        NextRecordIndex = 0;
        ChartData->clear();
    }
}
