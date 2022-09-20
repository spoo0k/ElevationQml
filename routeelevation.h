#ifndef QWTCASTOMPLOT_H
#define QWTCASTOMPLOT_H

#include <QtWidgets>
#include <QGeoPath>
#include "RouteTools/elevationtools.h"
#include "Elevation/elevation.h"

class QwtPlotPanner;
class QwtPlotCurve;
class QwtPlot;
class QwtPlotIntervalCurve;
class QwtPlotMarker;
class QwtPlotMagnifier;

class Route;
typedef QVector<QPointF> PlotSamples;

class PlotPicker;

class RouteElevation : public QObject
{
    Q_OBJECT
public:
    //    QwtCastomPlot();
//    RouteElevation(QQuickItem* parent = nullptr);
    RouteElevation(QWidget *parent = nullptr);
   ~RouteElevation();
    QwtPlot * getPlot(){return plot;}
public slots:
    void generateSamples();
    void setEditable( const bool editable );
    void updateRelief();
    void onUpdateRelief();
    void showPlotMarkers( bool show );
    void routeChanged(Route * route);

signals:
    void editableChanged( const bool editable );
    void routeIsChanged();

protected:
    void mouseReleaseEvent( QMouseEvent * event );
    void showEvent(QShowEvent * event);
    void initCurves();

protected slots:
    void initToolBar();
    void privatePointMoved( const QPoint & point );
    void acceptNewRelief();
    void onPickerBegin();
    void onPickerEnd();
    void slotProgressBuildRouteAndElevationProfiles(quint8 progress, const Elevation::RouteAndElevationProfiles &deltaResult);

private:

    Elevation::ElevationTools * elevationTools;
    Elevation::Elevation * elevation;
    QVector<Elevation::Point> m_finalRoute;

    bool m_showMarkers;
    PlotSamples ps;

    QToolBar * toolBar;
    QSpinBox * sbAltitude;
    QSpinBox * sbElevInterval;
    QDoubleSpinBox * sbVertSpeedUp;
    QDoubleSpinBox * sbVertSpeedDown;
    QSpinBox * sbVelocity;
    QAction * aCalc;
    QAction * aClean;
    QAction * aAccept;

    QwtPlot * plot;


    QwtPlotCurve * errorDots;
    QwtPlotCurve * routeCurve;
    QwtPlotCurve * routeBackgroundDots;
    QwtPlotCurve * routeDots;
    QwtPlotCurve * routeNewDots;

    QwtPlotCurve * reliefCurve;
    QwtPlotIntervalCurve * elevInterval;
    QwtPlotMagnifier * magnifier;
    Route * m_route;
    QGeoPath m_routeGeoPath;
    QList<QwtPlotMarker*> routePointMarkers;

    QPolygonF m_acceptedArea;

    bool buildRelief;

    PlotPicker * picker;

    bool pointMoving;
    QwtPlotCurve * newPointCurve;
    QwtPlotPanner * panner;
    QPointF newPoint;
    int newPointIndex;

    int indexOfMovingPoint;

    bool editable;



    void paint(QPainter* painter);


//    Q_INVOKABLE void initQwtPlot();

//private:
//    QwtPlot* plot;




//    void replotAndUpdate();

//private slots:
//    void updatePlotSize();

};
#endif // QWTCASTOMPLOT_H
