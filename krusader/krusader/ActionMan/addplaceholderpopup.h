//
// C++ Interface: addplaceholderpopup
//
// Description: 
//
//
// Author: Shie Erlich and Rafi Yanai <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef ADDPLACEHOLDERPOPUP_H
#define ADDPLACEHOLDERPOPUP_H

#include <kpopupmenu.h>
#include <kdialogbase.h>
#include "../UserAction/expander.h"

class QString;
class KLineEdit;
class QToolButton;
class QCheckBox;
class KComboBox;
class KrBookmarkButton;
class KURL;
class KIntSpinBox;


/**
 * This reads Expander::placeholder[] and fills a popup for easy access to the UserAction Placeholder
 * @author Jonas B�hr (http://www.jonas-baehr.de), Shie Erlich
 */
class AddPlaceholderPopup : public KPopupMenu {

public:
   AddPlaceholderPopup( QWidget *parent );
   
   /**
    * Use this to exec the popup. 
    * @param pos Position where the popup should appear
    * @return the expression which can be placed in the UserAction commandline
    */
   QString getPlaceholder( const QPoint& pos );

protected:
   /**
    * This is calles when a Placeholder got parameter.
    * @param currentPlaceholder A pointer to the Placeholder the user has choosen
    * @return a parameter-string
    */
   QString getParameter( exp_placeholder* currentPlaceholder );
   
private:
   KPopupMenu *_activeSub, *_otherSub, *_leftSub, *_rightSub, *_independentSub;
};


////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Parameter Widgets ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

/**
 *  abstract baseclass for all Parameter widgets
 * @author Jonas B�hr (http://www.jonas-baehr.de)
 */
class ParameterBase : public QWidget {
public:
   inline ParameterBase( const exp_parameter& parameter, QWidget* parent ) : QWidget( parent ) { _nessesary = parameter.nessesary(); }
   /**
    * @return the text for the parameter
    */
   virtual QString text() = 0;
   /**
    * @return the default of the parameter
    */
   virtual QString preset() = 0;
   /**
    * re-init the parameter with the default
    */
   virtual void reset() = 0;
   /**
    * @return true if the Parameter as a valid value
    */
   virtual bool valid() = 0;
   /**
    * @return true if the Placeholder realy needs this parameter
    */
   inline bool nessesary() { return _nessesary; }
private:
   bool _nessesary;
};

/**
 *  The simple Parameter widgets: a line-edit with the description above
 *  used by default
 */
class ParameterText : public ParameterBase {
public:
   ParameterText( const exp_parameter& parameter, QWidget* parent );
   QString text();
   QString preset();
   void reset();
   bool valid();
private:
   KLineEdit * _lineEdit;
   QString _preset;
};

/**
 *  A line-edit with the "addPlaceholder"-button
 *  used with default = "__placeholder"
 */
class ParameterPlaceholder : public ParameterBase {
Q_OBJECT
public:
   ParameterPlaceholder( const exp_parameter& parameter, QWidget* parent );
   QString text();
   QString preset();
   void reset();
   bool valid();
private:
   KLineEdit * _lineEdit;
   QToolButton* _button;
private slots:
   void addPlaceholder();
};

/**
 *  A Checkbox, default: checked; retuns "No" if unchecked
 *  used with default = "__yes"
 */
class ParameterYes : public ParameterBase {
public:
   ParameterYes( const exp_parameter& parameter, QWidget* parent );
   QString text();
   QString preset();
   void reset();
   bool valid();
private:
   QCheckBox* _checkBox;
};

/**
 *  A Checkbox, default: unchecked; retuns "Yes" if checked
 *  used with default = "__no"
 */
class ParameterNo : public ParameterBase {
public:
   ParameterNo( const exp_parameter& parameter, QWidget* parent );
   QString text();
   QString preset();
   void reset();
   bool valid();
private:
   QCheckBox* _checkBox;
};

/**
 *  A line-edit with the "file open"-button
 *  used with default = "__file"
 */
class ParameterFile : public ParameterBase {
Q_OBJECT
public:
   ParameterFile( const exp_parameter& parameter, QWidget* parent );
   QString text();
   QString preset();
   void reset();
   bool valid();
private:
   KLineEdit * _lineEdit;
   QToolButton* _button;
private slots:
   void addFile();
};

/**
 *  A ComboBox with the description above
 *  used with default = "__choose:item1;item2;..."
 */
class ParameterChoose : public ParameterBase {
public:
   ParameterChoose( const exp_parameter& parameter, QWidget* parent );
   QString text();
   QString preset();
   void reset();
   bool valid();
private:
   KComboBox * _combobox;
};

/**
 *  An editable ComboBox with the predifined selections
 *  used with default = "__select"
 */
class ParameterSelect : public ParameterBase {
public:
   ParameterSelect( const exp_parameter& parameter, QWidget* parent );
   QString text();
   QString preset();
   void reset();
   bool valid();
private:
   KComboBox * _combobox;
};

/**
 *  A line-edit with a "choose dir"- and a bookmark-button
 *  used with default = "__goto"
 */
class ParameterGoto : public ParameterBase {
Q_OBJECT
public:
   ParameterGoto( const exp_parameter& parameter, QWidget* parent );
   QString text();
   QString preset();
   void reset();
   bool valid();
private:
   KLineEdit * _lineEdit;
   QToolButton* _dirButton, *_placeholderButton;
private slots:
   void setDir();
   void addPlaceholder();
};

/**
 *  A ComboBox with all profiles available for the Synchronizer
 *  used with default = "__syncprofile"
 */
class ParameterSyncprofile : public ParameterBase {
public:
   ParameterSyncprofile( const exp_parameter& parameter, QWidget* parent );
   QString text();
   QString preset();
   void reset();
   bool valid();
private:
   KComboBox * _combobox;
};

/**
 *  A ComboBox with all profiles available for the panels
 *  used with default = "__panelprofile"
 */
class ParameterPanelprofile : public ParameterBase {
public:
   ParameterPanelprofile( const exp_parameter& parameter, QWidget* parent );
   QString text();
   QString preset();
   void reset();
   bool valid();
private:
   KComboBox * _combobox;
};

/**
 *  A ComboBox with all profiles available for the Searchmodule
 *  used with default = "__searchprofile"
 */
class ParameterSearch : public ParameterBase {
public:
   ParameterSearch( const exp_parameter& parameter, QWidget* parent );
   QString text();
   QString preset();
   void reset();
   bool valid();
private:
   KComboBox * _combobox;
};

/**
 *  A SpinBox for integer
 *  used with default = "__int:min;max;step;value"
 */
class ParameterInt : public ParameterBase {
public:
   ParameterInt( const exp_parameter& parameter, QWidget* parent );
   QString text();
   QString preset();
   void reset();
   bool valid();
private:
   KIntSpinBox * _spinbox;
   int _default;
};

////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// ParameterDialog ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

/**
 *  Opens a dialog for the parameter. Depending on the default (preset) a differend widget is used.
 *  See Parameter-Classes for details
 */
class ParameterDialog : public KDialogBase {
Q_OBJECT
public:
   ParameterDialog( const exp_placeholder* currentPlaceholder, QWidget *parent );
   
   /**
    * Use this to execute the dialog.
    * @return a QString with all paremeters; ommiting the optional ones if they have the default-value.
    */
   QString getParameter();

private:
   typedef QValueList<ParameterBase*> ParameterList;
   ParameterList _parameter;
   int _parameterCount;
private slots:
   void reset();
   void slotOk();
};


#endif // ADDPLACEHOLDERPOPUP_H
