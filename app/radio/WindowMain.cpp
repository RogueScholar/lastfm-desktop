
#include <QShortcut>
#include <QToolBar>

#include "WindowMacro.h"

#include "WindowMain.h"
#include "ui_WindowMain.h"

#include "widgets/PlayableItemWidget.h"
#include "Actions.h"

#include "../Radio.h"
#include "../StationSearch.h"

#include <lastfm/XmlQuery>
#include <lastfm/User>

WindowMain::WindowMain( Actions& actions ) :
    unicorn::MainWindow(),
    ui(new Ui::WindowMain),
    m_actions( &actions )
{
    SETUP()

#ifdef Q_OS_MAC
    QToolBar* toolbar = addToolBar(tr(""));
    toolbar->addWidget( ui->quickstartFrame );
    ui->quickstartFrame->setFrameStyle( QFrame::NoFrame );
    ui->quickstartFrame->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
#endif

    ui->stationEdit->setHelpText( tr("Type an artist or tag and press play") );

    connect( radio, SIGNAL(tuningIn(RadioStation)), SLOT(onTuningIn(RadioStation)));

    connect( ui->start, SIGNAL(clicked()), SLOT(onStartClicked()));
    connect( ui->stationEdit, SIGNAL(returnPressed()), SLOT(onStartClicked()));

    connect( ui->recent, SIGNAL(clicked()), SLOT(onRecentClicked()));
    connect( ui->library, SIGNAL(clicked()), SLOT(onLibraryClicked()));
    connect( ui->mix, SIGNAL(clicked()), SLOT(onMixClicked()));
    connect( ui->recommended, SIGNAL(clicked()), SLOT(onRecommendedClicked()));
    connect( ui->friends, SIGNAL(clicked()), SLOT(onFriendsClicked()));
    connect( ui->neighbours, SIGNAL(clicked()), SLOT(onNeighboursClicked()));

    // fetch the recent radio stations
    lastfm::User currentUser;
    connect( currentUser.getRecentStations(), SIGNAL(finished()), SLOT(onGotRecentStations()));
    connect( currentUser.getTopArtists( "3month", 9 ), SIGNAL(finished()), SLOT(onGotTopArtists()));
    onGotMixStations();
    connect( currentUser.getRecommendedArtists( 9 ), SIGNAL(finished()), SLOT(onGotRecommendedArtists()));
    connect( currentUser.getFriendsListeningNow( 9 ), SIGNAL(finished()), SLOT(onGotFriendsListeningNow()));
    connect( currentUser.getNeighbours( 9 ), SIGNAL(finished()), SLOT(onGotNeighbours()));

    // always start on the recent tab
    ui->recent->click();

    new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_M ), this, SLOT(onSwitch()));
}


void
WindowMain::addWinThumbBarButtons( QList<QAction*>& thumbButtonActions )
{
    thumbButtonActions.append( m_actions->m_loveAction );
    thumbButtonActions.append( m_actions->m_banAction );
    thumbButtonActions.append( m_actions->m_playAction );
    thumbButtonActions.append( m_actions->m_skipAction );
}


void
WindowMain::onActionsChanged()
{
    ui->love->setChecked( m_actions->m_loveAction->isChecked() );
    ui->ban->setChecked( m_actions->m_banAction->isChecked() );
    ui->play->setChecked( m_actions->m_playAction->isChecked() );
    ui->skip->setChecked( m_actions->m_skipAction->isChecked() );
}


WindowMain::~WindowMain()
{
    delete ui;
}


void
WindowMain::onStartClicked()
{
    QString trimmedText = ui->stationEdit->text().trimmed();
    if( trimmedText.startsWith("lastfm://")) {
        radio->play( RadioStation( trimmedText ) );
        return;
    }

    if (ui->stationEdit->text().length()) {
        StationSearch* s = new StationSearch();
        connect(s, SIGNAL(searchResult(RadioStation)), radio, SLOT(play(RadioStation)));
        s->startSearch(ui->stationEdit->text());
    }
}

void
WindowMain::onPlayClicked( bool checked )
{
    PLAY_CLICKED()
}


void
WindowMain::onLoveClicked( bool loved )
{
    LOVE_CLICKED()
}

void
WindowMain::onLoveTriggered()
{
    LOVE_TRIGGERED()
}

void
WindowMain::onBanClicked()
{
    BAN_CLICKED()
}

void
WindowMain::onBanFinished()
{
    BAN_FINISHED()
}


void
WindowMain::onRadioTick( qint64 tick )
{
    RADIO_TICK()
}


void
WindowMain::onTuningIn( const RadioStation& station )
{
    ON_TUNING_IN()

    // check if the widget is already in the recent list
    //
    bool found = false;

    for ( int i = 0 ; i < ui->recentLayout->count() ; ++i )
    {
        PlayableItemWidget* item = qobject_cast<PlayableItemWidget*>( ui->recentLayout->itemAt( i )->widget() );
        if ( item )
        {
            if ( item->station() == station )
            {
                // The station is already in the list
                // so move it to the front
                ui->recentLayout->insertWidget( 0, ui->recentLayout->takeAt( i )->widget() );
                found = true;
                break;
            }
        }
    }

    if ( !found )
    {
        ui->recentLayout->takeAt( ui->recentLayout->count() - 1 )->widget()->deleteLater();;

        PlayableItemWidget* item = new PlayableItemWidget( station );
        ui->recentLayout->insertWidget( 0, item );
        item->onTuningIn( station );
    }
}


void
WindowMain::onTrackSpooled( const Track& track )
{
    TRACK_SPOOLED()
}

void
WindowMain::onSwitch()
{
    emit aboutToHide();
    hide();
}

void
WindowMain::onRecentClicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void
WindowMain::onLibraryClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void
WindowMain::onMixClicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void
WindowMain::onRecommendedClicked()
{
    ui->stackedWidget->setCurrentIndex(3);
}

void
WindowMain::onFriendsClicked()
{
    ui->stackedWidget->setCurrentIndex(4);
}

void
WindowMain::onNeighboursClicked()
{
    ui->stackedWidget->setCurrentIndex(5);
}

void
WindowMain::onGotRecentStations()
{
    lastfm::XmlQuery lfm = lastfm::XmlQuery( static_cast<QNetworkReply*>( sender() )->readAll() );

    foreach ( const lastfm::XmlQuery& station , lfm.children("station") )
    {
        RadioStation recentStation( station["url"].text() );
        PlayableItemWidget* stationWidget = new PlayableItemWidget( station["name"].text(), recentStation );
        ui->recentLayout->addWidget(stationWidget);
        connect( stationWidget, SIGNAL(startRadio(RadioStation)), radio, SLOT(play(RadioStation)) );
    }
}

void
WindowMain::onGotTopArtists()
{
    lastfm::XmlQuery lfm = lastfm::XmlQuery( static_cast<QNetworkReply*>( sender() )->readAll() );

    PlayableItemWidget* recentStationWidget = new PlayableItemWidget( tr("My Library"), RadioStation::library( lastfm::User() ) );
    ui->libraryLayout->addWidget(recentStationWidget);
    connect( recentStationWidget, SIGNAL(startRadio(RadioStation)), radio, SLOT(play(RadioStation)) );

    ui->libraryLayout->addWidget( new QLabel( tr("Containing these artists"), this ) );

    foreach ( lastfm::XmlQuery artist, lfm.children("artist") )
    {
        PlayableItemWidget* stationWidget = new PlayableItemWidget( tr("%1 Radio").arg( artist["name"].text() ), RadioStation::similar( lastfm::Artist( artist["name"].text() ) ) );
        ui->libraryLayout->addWidget(stationWidget);
        connect( stationWidget, SIGNAL(startRadio(RadioStation)), radio, SLOT(play(RadioStation)) );
    }
}

void
WindowMain::onGotMixStations()
{
    //lastfm::XmlQuery lfm = lastfm::XmlQuery( static_cast<QNetworkReply*>( sender() )->readAll() );

    PlayableItemWidget* stationWidget = new PlayableItemWidget( tr("My Mix Radio"), RadioStation::mix( lastfm::User() ) );
    ui->mixLayout->addWidget(stationWidget);
    connect( stationWidget, SIGNAL(startRadio(RadioStation)), radio, SLOT(play(RadioStation)) );

    ui->mixLayout->addWidget( new QLabel( tr("Containing these artists"), this ) );
}

void
WindowMain::onGotRecommendedArtists()
{
    lastfm::XmlQuery lfm = lastfm::XmlQuery( static_cast<QNetworkReply*>( sender() )->readAll() );

    PlayableItemWidget* stationWidget = new PlayableItemWidget( tr("My Recommendations"), RadioStation::recommendations( lastfm::User() ) );
    ui->recLayout->addWidget(stationWidget);
    connect( stationWidget, SIGNAL(startRadio(RadioStation)), radio, SLOT(play(RadioStation)) );

    ui->recLayout->addWidget( new QLabel( tr("Containing these artists"), this ) );

    foreach ( lastfm::XmlQuery artist, lfm.children("artist") )
    {
        PlayableItemWidget* stationWidget = new PlayableItemWidget( tr("%1 Radio").arg( artist["name"].text() ), RadioStation::similar( lastfm::Artist( artist["name"].text() ) ) );
        ui->recLayout->addWidget(stationWidget);
        connect( stationWidget, SIGNAL(startRadio(RadioStation)), radio, SLOT(play(RadioStation)) );
    }
}

void
WindowMain::onGotFriendsListeningNow()
{
    lastfm::XmlQuery lfm = lastfm::XmlQuery( static_cast<QNetworkReply*>( sender() )->readAll() );

    PlayableItemWidget* recentStationWidget = new PlayableItemWidget( tr("My Friends' Radio"), RadioStation::friends( lastfm::User() ) );
    ui->friendsLayout->addWidget(recentStationWidget);
    connect( recentStationWidget, SIGNAL(startRadio(RadioStation)), radio, SLOT(play(RadioStation)) );

    ui->friendsLayout->addWidget( new QLabel( tr("Containing these users"), this ) );

    foreach ( lastfm::XmlQuery user, lfm.children("user") )
    {
        PlayableItemWidget* stationWidget = new PlayableItemWidget( tr("%1's Library").arg( user["name"].text() ), RadioStation::library( lastfm::User( user["name"].text() ) ) );
        ui->friendsLayout->addWidget(stationWidget);
        connect( stationWidget, SIGNAL(startRadio(RadioStation)), radio, SLOT(play(RadioStation)) );
    }
}

void
WindowMain::onGotNeighbours()
{
    lastfm::XmlQuery lfm = lastfm::XmlQuery( static_cast<QNetworkReply*>( sender() )->readAll() );

    PlayableItemWidget* recentStationWidget = new PlayableItemWidget( tr("My Neighbours' Radio"), RadioStation::neighbourhood( lastfm::User() ) );
    ui->neighboursLayout->addWidget(recentStationWidget);
    connect( recentStationWidget, SIGNAL(startRadio(RadioStation)), radio, SLOT(play(RadioStation)) );

    ui->neighboursLayout->addWidget( new QLabel( tr("Containing these users"), this ) );

    foreach ( lastfm::XmlQuery user, lfm.children("user") )
    {
        PlayableItemWidget* stationWidget = new PlayableItemWidget( tr("%1's Library").arg( user["name"].text() ), RadioStation::library( lastfm::User( user["name"].text() ) ) );
        ui->neighboursLayout->addWidget(stationWidget);
        connect( stationWidget, SIGNAL(startRadio(RadioStation)), radio, SLOT(play(RadioStation)) );
    }
}