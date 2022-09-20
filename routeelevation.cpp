#include "routeelevation.h"

//RouteElevation::RouteElevation(QQuickItem* parent): QQuickPaintedItem(parent){

//}

//void RouteElevation::paint(QPainter* painter)
//{
//    if (plot) {
//        QPixmap picture(boundingRect().size().toSize());

//        QwtPlotRenderer renderer;
//        renderer.renderTo(plot, picture);

//        painter->drawPixmap(QPoint(), picture);
//    }
//}

//void RouteElevation::initQwtPlot()
//{
//    plot = new QwtPlot();
//    plot->setAutoReplot(false);
//}


//#include "routeelevationwidget.h"

#include "plotpicker.h"

#include <QGeoPath>
#include <qwt_plot_curve.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_plot.h>
#include <QAction>
#include <QVBoxLayout>
#include <QToolBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QApplication>
#include <QPair>
#include <QRandomGenerator>
#include "../route.h"
#include <QDesktopWidget>

 inline void initMyResource() { Q_INIT_RESOURCE(elevationprofileresources); }

    RouteElevation::RouteElevation( QWidget* parent )
        : QMainWindow(parent)
        , elevationTools(new Elevation::ElevationTools(this))
        , elevation(new Elevation::Elevation(this))
        , m_showMarkers( false )
        , toolBar(new QToolBar)
        ,sbAltitude(new QSpinBox(toolBar))
        ,sbElevInterval(new QSpinBox(toolBar))
        ,sbVertSpeedUp(new QDoubleSpinBox(toolBar))
        ,sbVertSpeedDown(new QDoubleSpinBox(toolBar))
        ,sbVelocity(new QSpinBox(toolBar))
        ,aCalc(new QAction(toolBar))
        ,aClean(new QAction(toolBar))
        ,aAccept(new QAction(toolBar))
        , plot( new QwtPlot() )
        , errorDots(new QwtPlotCurve( tr("Error dots") ))
        , routeCurve( new QwtPlotCurve( tr("Route") ) )
        , routeBackgroundDots(new QwtPlotCurve( tr("Route") ))
        , routeDots( new QwtPlotCurve(tr("Route dots")))
        , routeNewDots( new QwtPlotCurve( tr("Route new dots")))
        , reliefCurve( new QwtPlotCurve( tr("Relief") ) )
        , elevInterval( new QwtPlotIntervalCurve( tr("Zero line") ) )
        , buildRelief(false)
        , picker( new PlotPicker(  plot->canvas() ) )
        , pointMoving(false)
        , newPointCurve(0)
        , indexOfMovingPoint(-1)
        , editable(true)
    {

        initMyResource();

        m_route = nullptr;

        setWindowFlag(Qt::SplashScreen);

        initToolBar();

        connect(elevationTools, &Elevation::ElevationTools::progressBuildRouteAndElevationProfiles, this, &RouteElevationWidget::slotProgressBuildRouteAndElevationProfiles);

        initCurves();
        //generateSamples();


        picker->setAxis(QwtPlot::xBottom,QwtPlot::yLeft);

        picker->setRubberBand(QwtPicker::CrossRubberBand);
        picker->setTrackerMode( QwtPicker::ActiveOnly );
        picker->setStateMachine( new QwtPickerDragPointMachine() );

        picker->setRubberBandPen(QColor::fromRgb(0,128,0));
        picker->setTrackerPen(QColor::fromRgb(0,0,255));
        QFont pFont = picker->trackerFont();
        pFont.setBold(true);
        picker->setTrackerFont(pFont);


        // Используйте колесо прокрутки для увеличения / уменьшения
        magnifier = new QwtPlotMagnifier( plot->canvas() );

        plot->setAxisScale(QwtPlot::yLeft, 0, 300);
        plot->setAxisScale(QwtPlot::xBottom, 0, 1000);
        plot->setAxisAutoScale(QwtPlot::xBottom, true);

        plot->setAxisAutoScale(QwtPlot::yLeft, true);



        magnifier->setAxisEnabled(QwtPlot::yLeft, false);
       // Используйте левую кнопку мыши для панорамирования
        panner =  new QwtPlotPanner( plot->canvas() );
        panner->setAxisEnabled(QwtPlot::yLeft, false);
        panner->setMouseButton(Qt::MiddleButton);

        QwtPlotGrid *grid = new QwtPlotGrid(); //
        grid->setMajorPen(QPen( Qt::gray, 1 )); // цвет линий и толщина
        grid->attach( plot ); // добавить сетку к полю графика
        plot->setCanvasBackground(QBrush(QColor("#87ceeb")));




    //    connect( picker, SIGNAL( moved( QPoint ) ), this, SLOT( privatePointMoved( QPoint ) ) );
    //    connect( picker, SIGNAL( beginPick() ), this, SLOT( onPickerBegin() ) );
    //    connect( picker, SIGNAL( endPick() ), this, SLOT( onPickerEnd() ) );

        setCentralWidget(plot);

        updateRelief();

    }

    RouteElevationWidget::~RouteElevationWidget()
    {

    }




    void RouteElevationWidget::updateRelief()
    {
        routeCurve->setSamples(ps);
        routeDots->setSamples(ps);
        routeBackgroundDots->setSamples(ps);

        QVector<QPointF> samp;
        for(int i = 0; i< ps.count(); i++){
            if (!m_acceptedArea.containsPoint(ps[i],Qt::OddEvenFill)){
                samp.append(ps[i]);
            }
        }
        errorDots->setSamples(samp);
        plot->replot();

    }

    void RouteElevationWidget::onUpdateRelief()
    {
        updateRelief();
    }

    void RouteElevationWidget::showPlotMarkers(bool show)
    {
        foreach ( QwtPlotMarker * marker, routePointMarkers )
        {
            marker->setVisible(show);
        }
        plot->update();
        plot->repaint();
    }

    void RouteElevationWidget::routeChanged(Route *route)
    {
        m_route = route;
        if (!isVisible()){
            return;
        }
        if (!m_route||m_route->pointsCount()<2){
            aCalc->setEnabled(false);
        } else {
            aCalc->setEnabled(true);
        }
        aAccept->setEnabled(false);
        m_routeGeoPath = m_route->geoPath();
        QList<QGeoCoordinate> routePath = m_routeGeoPath.path();

        elevInterval->setSamples(QVector<QwtIntervalSample>());
        errorDots->setSamples(QVector<QPointF>());
        routeNewDots->setSamples(QVector<QPointF>());

        QVector<QPointF> routeAltitudes;
        double dist = 0;
        QGeoCoordinate prevP;
        if (m_routeGeoPath.size()>0){
            prevP = routePath.at(0);
        }
        foreach(QGeoCoordinate point, routePath){
            dist += prevP.distanceTo(point);
            routeAltitudes.append(QPointF(dist,point.altitude()));
            prevP = point;
        }
        routeBackgroundDots->setSamples(routeAltitudes);
        routeDots->setSamples(routeAltitudes);
        routeCurve->setSamples(routeAltitudes);
        plot->axisAutoScale(1);
        plot->axisAutoScale(0);
        //Отображение рельефа и маршрута
        reliefCurve->setSamples(elevation->buildGroundProfileForChart(m_routeGeoPath));
        plot->replot();
    }



    void RouteElevationWidget::mouseReleaseEvent( QMouseEvent * event )
    {
        if (editable && pointMoving )
        {
            pointMoving = false;
        }
        else
        {
//            QWidget::mouseReleaseEvent(event);
        }


    }

    void RouteElevationWidget::showEvent(QShowEvent *event)
    {
        Q_UNUSED(event)
        if (m_route){
            routeChanged(m_route);
        }
    }




    void RouteElevationWidget::initCurves()
    {

        QPen pen;
        pen.setColor(Qt::blue);

        QColor bg(Qt::blue);
        bg.setAlpha(150);
        elevInterval->setPen(pen);
        elevInterval->setBrush(QBrush(bg));
        elevInterval->attach(plot);





        pen.setColor(Qt::black);
        pen.setWidth(1);
        reliefCurve->setBrush(QBrush(Qt::darkGray));
        reliefCurve->setPen(pen);
        reliefCurve->attach(plot);
        reliefCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

        pen.setColor("#FF7F50");
        pen.setWidth(2);
        routeCurve->setPen(pen);
        routeCurve->attach(plot);
        routeCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

        routeBackgroundDots->setStyle(QwtPlotCurve::Dots);
        pen.setColor(Qt::black);
        pen.setWidth(12);
        pen.setCapStyle(Qt::RoundCap);
        routeBackgroundDots->setPen(pen);
        routeBackgroundDots->attach(plot);
        routeBackgroundDots->setRenderHint(QwtPlotItem::RenderAntialiased, true);

        routeDots->setStyle(QwtPlotCurve::Dots);
        pen.setColor("#FF7F50");
        pen.setWidth(10);
        pen.setCapStyle(Qt::RoundCap);
        routeDots->setPen(pen);
        routeDots->attach(plot);
        routeDots->setRenderHint(QwtPlotItem::RenderAntialiased, true);

        routeNewDots->setStyle(QwtPlotCurve::Dots);
        pen.setColor(Qt::green);
        pen.setWidth(10);
        pen.setCapStyle(Qt::RoundCap);
        routeNewDots->setPen(pen);
        routeNewDots->attach(plot);
        routeNewDots->setRenderHint(QwtPlotItem::RenderAntialiased, true);

        errorDots->setStyle(QwtPlotCurve::Dots);
        pen.setColor(Qt::red);
        pen.setWidth(10);
        pen.setCapStyle(Qt::RoundCap);
        errorDots->setPen(pen);
        errorDots->attach(plot);
        errorDots->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    }

    void RouteElevationWidget::initToolBar()
    {

        sbAltitude->setMaximum(9999);
        //sbAltitude->
        sbAltitude->setValue(100);
        sbAltitude->setSingleStep(10);
        sbAltitude->setSuffix("м");
        sbElevInterval->setValue(10);
        sbElevInterval->setSuffix("м");
        sbVelocity->setValue(15);
        sbVelocity->setSuffix("м/c");
        sbVertSpeedUp->setValue(0.5);
        sbVertSpeedUp->setSuffix("м/c");
        sbVertSpeedDown->setValue(0.5);
        sbVertSpeedDown->setSuffix("м/c");

        QIcon icon = QIcon(":/images/calc_elevation.png");
//        icon.addFile(":/images/calc_elevation2.png",QSize(),QIcon::Selected);

        aCalc->setToolTip("Расчет огибания");
        aCalc->setIcon(icon);

        aAccept->setToolTip("Применить");
        aAccept->setIcon(QIcon(":/images/ok.png"));

        toolBar->addSeparator();
        toolBar->addWidget(new QLabel("Высота",this));
        toolBar->addWidget(sbAltitude);
        toolBar->addSeparator();
        toolBar->addWidget(new QLabel("Коридор",this));
        toolBar->addWidget(sbElevInterval);
        toolBar->addSeparator();
        toolBar->addWidget(new QLabel("V путевая",this));
        toolBar->addWidget(sbVelocity);
        toolBar->addSeparator();
        toolBar->addWidget(new QLabel("V подъема",this));
        toolBar->addWidget(sbVertSpeedUp);
        toolBar->addSeparator();
        toolBar->addWidget(new QLabel("V спуска",this));
        toolBar->addWidget(sbVertSpeedDown);
        toolBar->addSeparator();
        toolBar->addAction(aCalc);
        toolBar->addAction(aAccept);

        aCalc->setEnabled(false);
        aAccept->setEnabled(false);
        connect(aAccept,&QAction::triggered, this, &RouteElevationWidget::acceptNewRelief);
        connect(aCalc,&QAction::triggered,this,&RouteElevationWidget::generateSamples);

        addToolBar(Qt::TopToolBarArea, toolBar);
        toolBar->setFloatable(false);
        toolBar->setMovable(false);


    }



    void RouteElevationWidget::privatePointMoved( const QPoint & point )
    {
        if ( indexOfMovingPoint < 0 )
        {
            double dist;

            int index = routeDots->closestPoint(point, &dist);
            if( dist < 14 )
            {
                indexOfMovingPoint = index;
            }
        }

        if ( indexOfMovingPoint > -1  )
        {
            QwtScaleMap yMap = plot->canvasMap(QwtPlot::yLeft);

            const int baseScale = 10;
            if ( qAbs( yMap.s1() ) < baseScale && qAbs( yMap.s2() ) < baseScale )
            {
                yMap.setScaleInterval( -baseScale, baseScale );
            }


            double newY = yMap.invTransform( plot->canvas()->mapFromParent(point).y() );
            ps[indexOfMovingPoint].setY(newY);

            updateRelief();
        }
    }

    void RouteElevationWidget::acceptNewRelief()
    {
        int baseIndex = 0;

        foreach (Elevation::Point item, m_finalRoute){
            if (item.isBase()){
                RoutePoint point = m_route->point(baseIndex);
                point.altitude = item.altitude();
                point.velocity = sbVelocity->value();
                m_route->updatePoint(baseIndex, point);
            } else {
                RoutePoint point;
                point.latitude = item.latitude();
                point.longitude = item.longitude();
                point.altitude = item.altitude();
                point.velocity = sbVelocity->value();
                m_route->insertPoint(baseIndex, point);
            }
            baseIndex++;
        }
        routeChanged(m_route);
        emit routeIsChanged();
    }

    void RouteElevationWidget::onPickerBegin()
    {
        indexOfMovingPoint = -1;
    }

    void RouteElevationWidget::onPickerEnd()
    {
        updateRelief();
        indexOfMovingPoint = -1;
    }


    void RouteElevationWidget::generateSamples()
    {

        elevationTools->buildRouteAndElevationProfiles(m_routeGeoPath,
                                                       sbAltitude->value(),
                                                       sbElevInterval->value(),
                                                       sbVelocity->value(),
                                                       sbVertSpeedUp->value(),sbVertSpeedDown->value());


    }

    void RouteElevationWidget::slotProgressBuildRouteAndElevationProfiles(quint8 progress, const Elevation::RouteAndElevationProfiles &deltaResult) {
        qDebug()<<"progress - "<<progress;
        reliefCurve->setSamples(deltaResult.groundElevationProfile());
        m_acceptedArea.clear();
        QVector<QwtIntervalSample> intervals;
        for (int i = 0; i < deltaResult.lowBound().count(); i++){
            QwtIntervalSample sample(deltaResult.lowBound().at(i).x(),deltaResult.lowBound().at(i).y(),deltaResult.highBound().at(i).y());
            m_acceptedArea.append(deltaResult.lowBound().at(i));
            intervals<<sample;
        }
        int lastTwoPointsId = m_acceptedArea.count()-1;
        for (int i = deltaResult.highBound().count()-1; i >= 0; i--){
            m_acceptedArea.append(deltaResult.highBound().at(i));
        }
        QVector<QPointF> newDots;
        m_finalRoute = deltaResult.route();
        foreach(Elevation::Point item, deltaResult.route()){
            if (!item.isBase()){
                newDots.append(QPointF(item.distance(),item.altitude()));
            }

        }
        routeNewDots->setSamples(newDots);

        m_acceptedArea[lastTwoPointsId].setX(m_acceptedArea[lastTwoPointsId].x()+1);
        m_acceptedArea[lastTwoPointsId+1].setX(m_acceptedArea[lastTwoPointsId+1].x()+1);
        elevInterval->setSamples(intervals);

        ps = deltaResult.routeProfile();
        plot->axisAutoScale(1);
        aAccept->setEnabled(true);
        updateRelief();
    }

    void RouteElevationWidget::setEditable( const bool newEditable )
    {
        if ( newEditable == editable )
            return;
        editable = newEditable;
        emit editableChanged(editable);
    }


