#pragma once

#include <QObject>
#include <QApplication>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QColor>
#include <QList>
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QWidget>
#include <QTimer>

// Qt Charts includes
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QAbstractSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLegend>

QT_CHARTS_USE_NAMESPACE;

class ShadcnTheme : public QObject {
    Q_OBJECT;
public:
    enum Theme {
        Dark,
        Light
    };
    Q_ENUM(Theme)
    
    /**
     * Constructor
     * @param qssBasePath - Base path for QSS files (default: ":/")
     *                      Files should be named: shadcn-dark.qss and shadcn-light.qss
     */
    explicit ShadcnTheme(const QString& qssBasePath = ":/", QObject* parent = nullptr)
        : QObject(parent)
        , m_qssBasePath(qssBasePath)
        , m_currentTheme(Dark)
        , m_autoStyleEnabled(false)
        , m_monitoredWindow(nullptr)
    {
        if (!m_qssBasePath.endsWith('/')) {
            m_qssBasePath += '/';
        }
        
        // Timer for periodic chart discovery
        m_discoveryTimer = new QTimer(this);
        m_discoveryTimer->setInterval(500); // Check every 500ms
        connect(m_discoveryTimer, &QTimer::timeout, this, &ShadcnTheme::discoverAndStyleCharts);
    }
    
    /**
     * Install theme on a window - automatically styles all current and future charts
     * @param window - Window to monitor and style
     * @param theme - Initial theme to apply
     * @param app - QApplication instance (uses qApp if nullptr)
     */
    void installOnWindow(QWidget* window, Theme theme = Dark, QApplication* app = nullptr) {
        if (!window) return;
        
        m_monitoredWindow = window;
        m_currentTheme = theme;
        m_autoStyleEnabled = true;
        
        // Apply application stylesheet
        setTheme(theme, app);
        
        // Install event filter to catch new widgets
        window->installEventFilter(this);
        
        // Initial styling of existing charts
        discoverAndStyleCharts();
        
        // Start monitoring for new charts
        m_discoveryTimer->start();
    }
    
    /**
     * Uninstall theme from window
     */
    void uninstallFromWindow() {
        if (m_monitoredWindow) {
            m_monitoredWindow->removeEventFilter(this);
            m_monitoredWindow = nullptr;
        }
        m_discoveryTimer->stop();
        m_autoStyleEnabled = false;
        m_styledCharts.clear();
    }
    
    /**
     * Manually trigger styling of all charts in the monitored window
     */
    void refreshAllCharts() {
        if (m_monitoredWindow) {
            discoverAndStyleCharts();
        }
    }
    
    /**
     * Set the current theme and apply to application and all charts
     * @param theme - Theme to apply (Dark or Light)
     * @param app - QApplication instance (uses qApp if nullptr)
     */
    void setTheme(Theme theme, QApplication* app = nullptr) {
        m_currentTheme = theme;
        
        if (!app) {
            app = qApp;
        }
        
        QString qssPath = m_qssBasePath + (theme == Dark ? "shadcn-dark.css" : "shadcn-light.css");
        QString styleSheet = loadStyleSheet(qssPath);
        
        if (!styleSheet.isEmpty() && app) {
            app->setStyleSheet(styleSheet);
        }
        
        // Restyle all existing charts with new theme
        if (m_autoStyleEnabled && m_monitoredWindow) {
            m_styledCharts.clear(); // Force restyle
            discoverAndStyleCharts();
        }
        
        emit themeChanged(theme);
    }
    
    /**
     * Get current theme
     */
    Theme currentTheme() const {
        return m_currentTheme;
    }
    
    /**
     * Toggle between dark and light themes
     */
    void toggleTheme(QApplication* app = nullptr) {
        setTheme(m_currentTheme == Dark ? Light : Dark, app);
    }
    
    /**
     * Apply theme styling to a chart
     * @param chart - Chart to style
     * @param chartView - Optional chart view to style background
     * @param theme - Theme to apply (uses current theme if not specified)
     */
    void applyToChart(QChart* chart, QChartView* chartView = nullptr, Theme theme = Dark) {
        if (!chart) return;
        
        // Use current theme if default parameter
        if (theme == Dark && m_currentTheme == Light) {
            theme = Light;
        }
        
        if (theme == Dark) {
            applyDarkThemeToChart(chart, chartView);
        } else {
            applyLightThemeToChart(chart, chartView);
        }
        
        // Style all series in the chart
        styleAllSeries(chart, theme);
    }
    
    /**
     * Apply theme to chart using current theme
     */
    void applyToChart(QChart* chart, QChartView* chartView = nullptr) {
        applyToChart(chart, chartView, m_currentTheme);
    }
    
    /**
     * Style a series with automatic color selection
     * @param series - Series to style
     * @param colorIndex - Index in color palette (0-7, auto-increments if -1)
     * @param theme - Theme to use for styling
     */
    void styleSeries(QAbstractSeries* series, int colorIndex = -1, Theme theme = Dark) {
        if (!series) return;
        
        // Use current theme if default
        if (theme == Dark && m_currentTheme == Light) {
            theme = Light;
        }
        
        // Auto-increment color index
        static int autoColorIndex = 0;
        if (colorIndex < 0) {
            colorIndex = autoColorIndex++;
            if (autoColorIndex >= 8) autoColorIndex = 0;
        }
        
        QColor color = getChartPalette()[colorIndex % 8];
        
        // Style based on series type
        if (auto* lineSeries = qobject_cast<QLineSeries*>(series)) {
            styleLineSeries(lineSeries, color);
        }
        else if (auto* splineSeries = qobject_cast<QSplineSeries*>(series)) {
            styleSplineSeries(splineSeries, color);
        }
        else if (auto* areaSeries = qobject_cast<QAreaSeries*>(series)) {
            styleAreaSeries(areaSeries, color);
        }
        else if (auto* barSeries = qobject_cast<QBarSeries*>(series)) {
            styleBarSeries(barSeries, colorIndex);
        }
        else if (auto* pieSeries = qobject_cast<QPieSeries*>(series)) {
            stylePieSeries(pieSeries, theme == Dark);
        }
        else if (auto* scatterSeries = qobject_cast<QScatterSeries*>(series)) {
            styleScatterSeries(scatterSeries, color);
        }
    }

    /**
     * Style all series in a chart automatically
     * @param chart - Chart containing series to style
     * @param theme - Theme to use
     */
    void styleAllSeries(QChart* chart, Theme theme = Dark) {
        if (!chart) return;

        // Use current theme if default
        if (theme == Dark && m_currentTheme == Light) {
            theme = Light;
        }

        QList<QAbstractSeries*> seriesList = chart->series();
        for (int i = 0; i < seriesList.size(); ++i) {
            styleSeries(seriesList[i], i, theme);
        }
    }

    /**
     * Get the chart color palette
     */
    static QList<QColor> getChartPalette() {
        return {
            QColor("#3b82f6"),  // Blue
            QColor("#ef4444"),  // Red
            QColor("#10b981"),  // Green
            QColor("#f59e0b"),  // Amber
            QColor("#8b5cf6"),  // Violet
            QColor("#ec4899"),  // Pink
            QColor("#06b6d4"),  // Cyan
            QColor("#84cc16")   // Lime
        };
    }

    /**
     * Get a specific color from the palette
     */
    static QColor getChartColor(int index) {
        QList<QColor> palette = getChartPalette();
        return palette[index % palette.size()];
    }

    /**
     * Get theme colors structure
     */
    struct ThemeColors {
        QColor background;
        QColor foreground;
        QColor muted;
        QColor mutedForeground;
        QColor border;
        QColor legendBackground;
        QColor gridLine;
        QColor axisLine;
    };

    /**
     * Get colors for a specific theme
     */
    static ThemeColors getThemeColors(Theme theme) {
        ThemeColors colors;

        if (theme == Dark) {
            colors.background = QColor("#09090b");
            colors.foreground = QColor("#fafafa");
            colors.muted = QColor("#27272a");
            colors.mutedForeground = QColor("#a1a1aa");
            colors.border = QColor("#27272a");
            colors.legendBackground = QColor("#18181b");
            colors.gridLine = QColor("#27272a");
            colors.axisLine = QColor("#3f3f46");
        } else {
            colors.background = QColor("#ffffff");
            colors.foreground = QColor("#09090b");
            colors.muted = QColor("#f4f4f5");
            colors.mutedForeground = QColor("#71717a");
            colors.border = QColor("#e4e4e7");
            colors.legendBackground = QColor("#f4f4f5");
            colors.gridLine = QColor("#e4e4e7");
            colors.axisLine = QColor("#d4d4d8");
        }

        return colors;
    }

signals:
    void themeChanged(Theme theme);
    void chartStyled(QChart* chart);

protected:
    /**
     * Event filter to catch child added events
     */
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::ChildAdded || 
            event->type() == QEvent::ChildPolished) {
            // Trigger chart discovery when new children are added
            QTimer::singleShot(100, this, &ShadcnTheme::discoverAndStyleCharts);
        }
        return QObject::eventFilter(watched, event);
    }

private slots:
    /**
     * Discover and style all charts in the monitored window
     */
    void discoverAndStyleCharts() {
        if (!m_monitoredWindow || !m_autoStyleEnabled) {
            return;
        }
        
        // Find all QChartView widgets
        QList<QChartView*> chartViews = m_monitoredWindow->findChildren<QChartView*>();
        
        for (QChartView* chartView : chartViews) {
            QChart* chart = chartView->chart();
            if (chart && !m_styledCharts.contains(chart)) {
                // New chart found - style it
                applyToChart(chart, chartView, m_currentTheme);
                m_styledCharts.insert(chart);
                
                // Connect to chart destroyed signal to clean up tracking
                connect(chart, &QObject::destroyed, this, [this, chart]() {
                    m_styledCharts.remove(chart);
                });
                
                emit chartStyled(chart);
            }
        }
    }

private:
    QString loadStyleSheet(const QString& path) {
        QFile file(path);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            qWarning("Failed to load stylesheet: %s", qPrintable(path));
            return QString();
        }

        QTextStream stream(&file);
        return stream.readAll();
    }

    void applyDarkThemeToChart(QChart* chart, QChartView* chartView) {
        ThemeColors colors = getThemeColors(Dark);

        // Chart view background
        if (chartView) {
            chartView->setBackgroundBrush(QBrush(colors.background));
        }

        // Chart background
        chart->setBackgroundBrush(QBrush(colors.background));
        chart->setPlotAreaBackgroundBrush(QBrush(colors.background));
        chart->setPlotAreaBackgroundVisible(true);

        // Title
        chart->setTitleBrush(QBrush(colors.foreground));
        QFont titleFont = chart->titleFont();
        titleFont.setPointSize(16);
        titleFont.setBold(true);
        chart->setTitleFont(titleFont);

        // Legend
        QLegend* legend = chart->legend();
        if (legend) {
            legend->setBackgroundVisible(true);
            legend->setBorderColor(colors.border);
            legend->setLabelColor(colors.foreground);

            QFont legendFont = legend->font();
            legendFont.setPointSize(10);
            legend->setFont(legendFont);
        }

        // Axes
        styleAxes(chart, colors);

        chart->setMargins(QMargins(16, 16, 16, 16));
    }

    void applyLightThemeToChart(QChart* chart, QChartView* chartView) {
        ThemeColors colors = getThemeColors(Light);

        // Chart view background
        if (chartView) {
            chartView->setBackgroundBrush(QBrush(colors.background));
        }

        // Chart background
        chart->setBackgroundBrush(QBrush(colors.background));
        chart->setPlotAreaBackgroundBrush(QBrush(colors.background));
        chart->setPlotAreaBackgroundVisible(true);

        // Title
        chart->setTitleBrush(QBrush(colors.foreground));
        QFont titleFont = chart->titleFont();
        titleFont.setPointSize(16);
        titleFont.setBold(true);
        chart->setTitleFont(titleFont);

        // Legend
        QLegend* legend = chart->legend();
        if (legend) {
            legend->setBackgroundVisible(true);
            legend->setBorderColor(colors.border);
            legend->setLabelColor(colors.foreground);

            QFont legendFont = legend->font();
            legendFont.setPointSize(10);
            legend->setFont(legendFont);
        }

        // Axes
        styleAxes(chart, colors);

        chart->setMargins(QMargins(16, 16, 16, 16));
    }

    void styleAxes(QChart* chart, const ThemeColors& colors) {
        for (QAbstractAxis* axis : chart->axes()) {
            // Labels
            axis->setLabelsColor(colors.mutedForeground);
            QFont axisFont = axis->labelsFont();
            axisFont.setPointSize(9);
            axis->setLabelsFont(axisFont);

            // Grid lines
            axis->setGridLineColor(colors.gridLine);
            QPen gridPen(colors.gridLine);
            gridPen.setStyle(Qt::DotLine);
            axis->setGridLinePen(gridPen);

            // Axis line
            axis->setLinePen(QPen(colors.axisLine));

            // Axis title
            if (auto* valueAxis = qobject_cast<QValueAxis*>(axis)) {
                valueAxis->setTitleBrush(QBrush(colors.foreground));
                QFont titleFont = valueAxis->titleFont();
                titleFont.setPointSize(11);
                titleFont.setBold(true);
                valueAxis->setTitleFont(titleFont);
            }
            else if (auto* categoryAxis = qobject_cast<QBarCategoryAxis*>(axis)) {
                categoryAxis->setTitleBrush(QBrush(colors.foreground));
                QFont titleFont = categoryAxis->titleFont();
                titleFont.setPointSize(11);
                titleFont.setBold(true);
                categoryAxis->setTitleFont(titleFont);
            }
            else if (auto* dateTimeAxis = qobject_cast<QDateTimeAxis*>(axis)) {
                dateTimeAxis->setTitleBrush(QBrush(colors.foreground));
                QFont titleFont = dateTimeAxis->titleFont();
                titleFont.setPointSize(11);
                titleFont.setBold(true);
                dateTimeAxis->setTitleFont(titleFont);
            }
        }
    }

    void styleLineSeries(QLineSeries* series, const QColor& color) {
        QPen pen(color);
        pen.setWidth(2);
        series->setPen(pen);
    }

    void styleSplineSeries(QSplineSeries* series, const QColor& color) {
        QPen pen(color);
        pen.setWidth(2);
        series->setPen(pen);
    }

    void styleAreaSeries(QAreaSeries* series, const QColor& color) {
        QLinearGradient gradient(0, 0, 0, 1);
        gradient.setColorAt(0.0, color);
        gradient.setColorAt(1.0, color.darker(150));
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        series->setBrush(gradient);

        QPen pen(color);
        pen.setWidth(2);
        series->setPen(pen);
    }

    void styleBarSeries(QBarSeries* series, int startColorIndex) {
        QList<QColor> palette = getChartPalette();

        for (int i = 0; i < series->count(); ++i) {
            QBarSet* set = series->barSets()[i];
            QColor color = palette[(startColorIndex + i) % palette.size()];
            set->setColor(color);
            set->setBorderColor(color.darker(110));
        }
    }

    void stylePieSeries(QPieSeries* series, bool isDark) {
        QList<QColor> palette = getChartPalette();
        QColor borderColor = isDark ? QColor("#27272a") : QColor("#e4e4e7");

        for (int i = 0; i < series->count(); ++i) {
            QPieSlice* slice = series->slices()[i];
            slice->setColor(palette[i % palette.size()]);
            slice->setBorderColor(borderColor);
            slice->setBorderWidth(2);
        }
    }

    void styleScatterSeries(QScatterSeries* series, const QColor& color) {
        series->setColor(color);
        series->setBorderColor(color.darker(120));
        series->setMarkerSize(10);
    }

    QString m_qssBasePath;
    Theme m_currentTheme;
    bool m_autoStyleEnabled;
    QWidget* m_monitoredWindow;
    QTimer* m_discoveryTimer;
    QSet<QChart*> m_styledCharts;
};
