
#include <QEventLoop>
#include <QApplication>
#include <QToolTip>
#include <QTimer>
#include <QDebug>

#include "lib/unicorn/widgets/BannerWidget.h"
#include "lib/unicorn/widgets/HttpImageWidget.h"
#include "lib/unicorn/DesktopServices.h"

#include "WidgetTextObject.h"
#include "BioWidget.h"

BioWidget::BioWidget( QWidget* p ) 
    :QTextBrowser( p ),
      m_currentHoverWidget(0)
{
    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    connect(document()->documentLayout(), SIGNAL( documentSizeChanged(QSizeF)), SLOT( onBioChanged(QSizeF)));

    connect(this, SIGNAL(anchorClicked(QUrl)), SLOT(onAnchorClicked(QUrl)));

    m_widgetTextObject = new WidgetTextObject;
    viewport()->installEventFilter( this );
    document()->documentLayout()->registerHandler( WidgetImageFormat, m_widgetTextObject );

    ui.image = new HttpImageWidget(this);
    ui.image->setFixedWidth( 160 );
    ui.image->setAlignment( Qt::AlignTop );
    
    ui.onTour = new BannerWidget( tr("On Tour" ) );
    ui.onTour->setBannerVisible( false );
    ui.onTour->setWidget( ui.image );

    ui.onTour->setFixedWidth( 170 );
    ui.onTour->setObjectName( "onTour" );

    connect( ui.image, SIGNAL(loaded()), SLOT(onImageLoaded()));
    connect( ui.image, SIGNAL(loaded()), SLOT(update()));

    connect( this, SIGNAL(highlighted(QString)), SLOT(onHighlighted(QString)) );

    qDebug() << fontInfo().pixelSize();
}

void
BioWidget::setBioText( const QString& bioText )
{
    m_bioText = bioText;
}

void
BioWidget::onHighlighted( const QString& url )
{
    QUrl displayUrl( url );
    QToolTip::showText( cursor().pos(), displayUrl.toString(), this, QRect() );
}

void
BioWidget::onImageLoaded()
{
    qDebug() << ui.image->pixmap()->width() << ui.image->pixmap()->height();

    insertWidget( ui.onTour );
    QTimer::singleShot( 20, this, SLOT(appendBioText()) );
}

void
BioWidget::appendBioText()
{

    append( m_bioText );

    QTimer::singleShot( 20, this, SLOT(updateGeometryPlease()) );
}

void
BioWidget::updateGeometryPlease()
{
    // okay then
    updateGeometry();
}

void 
BioWidget::insertWidget( QWidget* w ) 
{
    w->installEventFilter( this );
    m_widgetImageFormat.setObjectType( WidgetImageFormat );
    m_widgetImageFormat.setProperty( WidgetData, QVariant::fromValue<QWidget*>( w ) );
    m_widgetImageFormat.setName( w->objectName() );
    
    QTextCursor cursor = textCursor();
    cursor.insertImage( m_widgetImageFormat, QTextFrameFormat::FloatLeft );
    setTextCursor( cursor );
}

bool 
BioWidget::eventFilter( QObject* o, QEvent* e )
{
    QWidget* w = qobject_cast<QWidget*>( o );

    if ( viewport() == w )
    {
        if ( QEvent::MouseMove != e->type() )
            return false;

        QMouseEvent* event = static_cast<QMouseEvent*>(e);
        //respect child widget cursor
        QWidget* w = m_widgetTextObject->widgetAtPoint(event->pos() );

        if ( w != m_currentHoverWidget )
        {
            m_currentHoverWidget = w;

            if( 0 == w )
                viewport()->unsetCursor();
            else
            {
                QWidget* c = w->childAt(event->pos());
                c = c ? c : w;
                viewport()->setCursor( c->cursor());
            }
        }

        return false;
    }
    return false;
}


void
BioWidget::mousePressEvent( QMouseEvent* event )
{
    update();
    if ( !sendMouseEvent(event) )
        QTextBrowser::mousePressEvent( event );
}


void
BioWidget::mouseReleaseEvent( QMouseEvent* event )
{
    if ( !sendMouseEvent(event) )
        QTextBrowser::mouseReleaseEvent( event );
}


void
BioWidget::mouseMoveEvent( QMouseEvent* event ) 
{
    if (!sendMouseEvent(event))
        QTextBrowser::mouseMoveEvent( event );
}

bool 
BioWidget::sendMouseEvent( QMouseEvent* event ) 
{
    QWidget* w = m_widgetTextObject->widgetAtPoint( event->pos());

    if ( !w )
        return false;

    QRectF wRect = m_widgetTextObject->widgetRect( w );
    QPoint pos = event->pos() - wRect.toRect().topLeft();

    QWidget* childWidget = w->childAt( event->pos());

    if( !childWidget )
        childWidget = w;
    else
        pos = childWidget->mapTo( w, pos );

    QMouseEvent* widgetMouseEvent = new QMouseEvent( event->type(), pos, event->button(), event->buttons(), event->modifiers());

    QCoreApplication::postEvent( childWidget, widgetMouseEvent );
    event->accept();

    return true;
}

void 
BioWidget::onAnchorClicked( const QUrl& link )
{
    unicorn::DesktopServices::openUrl( link );
}


void
BioWidget::onDocumentLayoutChanged()
{
    qDebug() << document()->size();

    setFixedHeight( document()->size().height() );
}

void 
BioWidget::onBioChanged( const QSizeF& size )
{
    qDebug() << size.toSize();
    qDebug() << document()->size();

    updateGeometry();

    QTimer::singleShot( 20, this, SLOT(onDocumentLayoutChanged()));
}

void
BioWidget::setPixmap( const QPixmap& pixmap )
{
    qDebug() << pixmap.height();

    ui.image->setPixmap( pixmap );
    //onImageLoaded();
}

void 
BioWidget::loadImage( const QUrl& url, HttpImageWidget::ScaleType scale )
{
    ui.image->loadUrl( url, scale );
}

void
BioWidget::setImageHref( const QUrl& href )
{
    ui.image->setHref( href );
}

void 
BioWidget::setOnTourVisible( bool visible, const QUrl& url )
{
    ui.onTour->setBannerVisible( visible );

    if ( url.isValid())
        ui.onTour->setHref( url );

    update();
}
