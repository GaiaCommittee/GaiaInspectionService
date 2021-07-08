#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QtCharts>

#include <string>
#include <memory>

#include <GaiaInspectionReader/GaiaInspectionReader.hpp>

namespace Gaia::InspectionChart
{
    QT_BEGIN_NAMESPACE
    namespace Ui { class ChartWindow; }
    QT_END_NAMESPACE

    class ChartWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        /**
         * @brief Connect to the given reader and build the window.
         * @param reader Reader for the inspected value.
         * @param parent Parent widget.
         */
        explicit ChartWindow(
                std::unique_ptr<InspectionService::InspectionReader>&& reader, QWidget *parent = nullptr);

        /// Release resources.
        ~ChartWindow() override;

    protected slots:
        /// Triggered when frequency spin changed.
        void OnFrequencyChanged(int value);
        /// Triggered when variable name changed.
        void OnVariableChanged(const QString& name);
        /// Triggered when update timer time out.
        void OnUpdate();

    private:
        std::string VariableName;

        unsigned long NextRecordIndex {0};

        /// Window resource.
        Ui::ChartWindow *ui;

        /// Inspected variable reader.
        std::unique_ptr<InspectionService::InspectionReader> Reader;
        /// Timer for auto update.
        QTimer* UpdateTimer {nullptr};
        /// Chart data for visualization.
        QChart* ChartModel {nullptr};
        /// Chart view for data visualization.
        QChartView* ChartView {nullptr};
        /// Data to visualize in the chart.
        QLineSeries* ChartData {nullptr};

        QValueAxis* AxisX {nullptr};
        QValueAxis* AxisY {nullptr};
    };
}