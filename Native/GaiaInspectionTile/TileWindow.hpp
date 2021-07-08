#pragma once

#include <QMainWindow>
#include <QTimer>
#include <string>
#include <memory>
#include <GaiaInspectionReader/GaiaInspectionReader.hpp>

namespace Gaia::InspectionTile
{
    QT_BEGIN_NAMESPACE
    namespace Ui { class TileWindow; }
    QT_END_NAMESPACE

    class TileWindow : public QMainWindow
    {
    Q_OBJECT

    public:
        /// Constructor which will bind the inspection variables reader.
        TileWindow(
                std::unique_ptr<InspectionService::InspectionReader>&& reader,
                std::string  variable_name,
                unsigned int update_frequency = 30,
                QWidget *parent = nullptr);
        /// Destructor which will release resources.
        ~TileWindow() override;

    protected slots:
        /// Update displayed value.
        void OnUpdate();

    private:
        /// Reader for inspected variables.
        std::unique_ptr<InspectionService::InspectionReader> Reader;

        /// Name of the variable to inspect.
        std::string VariableName;

        /// Point to UI object.
        Ui::TileWindow *ui;

        /// Timer for auto update.
        QTimer* UpdateTimer {nullptr};
    };
}