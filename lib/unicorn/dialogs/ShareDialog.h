/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

   This file is part of the Last.fm Desktop Application Suite.

   lastfm-desktop is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   lastfm-desktop is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with lastfm-desktop.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef SHARE_DIALOG_H
#define SHARE_DIALOG_H

#include <lastfm/Track>
#include <QDialogButtonBox>
#include "UnicornDialog.h"

#include "lib/DllExportMacro.h"

class UNICORN_DLLEXPORT ShareDialog : public unicorn::Dialog
{
    Q_OBJECT

    struct {
        class QRadioButton* trackShare;
        class QRadioButton* albumShare;
        class QRadioButton* artistShare;
        class TrackWidget* track;
        class RecipientsWidget* recipients;
        class QPlainTextEdit* message;
        class QCheckBox* isPublic;
        class QLabel* characterLimit;
        QDialogButtonBox* buttons;
    } ui;
    
public:
    ShareDialog( const Track&, QWidget* parent );

	Track track() const { return m_track; }

    void setupUi();

private slots:
    void enableDisableOk();
    void onRadioButtonsClicked( bool checked );
    void accept();
    void onMessageChanged();

private:
    class QPushButton* ok() { return ui.buttons->button( QDialogButtonBox::Ok ); }
    void updateCharacterLimit();

    Track m_track;
};

#endif