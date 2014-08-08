/***************************************************************************
* Copyright (c) 2014 Abdullah Tahiri <abdullah.tahiri.yo@gmail.com>        *
*                                                                          *
* This file is part of the FreeCAD CAx development system.                 *
*                                                                          *
* This library is free software; you can redistribute it and/or            *
* modify it under the terms of the GNU Library General Public              *
* License as published by the Free Software Foundation; either             *
* version 2 of the License, or (at your option) any later version.         *
*                                                                          *
* This library is distributed in the hope that it will be useful,          *
* but WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the             *
* GNU Library General Public License for more details.                     *
*                                                                          *
* You should have received a copy of the GNU Library General Public        *
* License along with this library; see the file COPYING.LIB. If not,       *
* write to the Free Software Foundation, Inc., 59 Temple Place,            *
* Suite 330, Boston, MA 02111-1307, USA                                    *
*                                                                          *
***************************************************************************/

#include "PreCompiled.h"

#ifndef _PreComp_
# include <algorithm>
# include <QComboBox>
# include <QFontDatabase>
# include <QLayout>
# include <QLocale>
# include <QPixmap>
# include <QSpinBox>
# include <QTextStream>
# include <QTimer>
#endif

#include <Base/Tools.h>
#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <App/PropertyGeo.h>
#include <App/PropertyFile.h>
#include <App/PropertyUnits.h>
#include <Gui/Application.h>
#include <Gui/Control.h>
#include <Gui/Document.h>
#include <Gui/Selection.h>
#include <Gui/ViewProviderDocumentObject.h>
#include <Gui/Placement.h>
#include <Gui/FileDialog.h>
#include <Gui/DlgPropertyLink.h>
#include <Gui/QuantitySpinBox.h>

#include <Gui/propertyeditor/PropertyItem.h>

#include "../App/PropertyConstraintList.h"

#include "PropertyConstraintListItem.h"


using namespace SketcherGui;
using namespace Gui::PropertyEditor;

TYPESYSTEM_SOURCE(SketcherGui::PropertyConstraintListItem, Gui::PropertyEditor::PropertyItem);

PropertyConstraintListItem::PropertyConstraintListItem()
{
    
}

QVariant PropertyConstraintListItem::toString(const QVariant& prop) const
{
    return prop;
}

QVariant PropertyConstraintListItem::value(const App::Property* prop) const
{
    assert(prop && prop->getTypeId().isDerivedFrom(Sketcher::PropertyConstraintList::getClassTypeId()));
    
    QString valuestr;
    fillInSubProperties(prop,valuestr);

    return QVariant(valuestr);
}

void PropertyConstraintListItem::setValue(const QVariant& value)
{
}

QWidget* PropertyConstraintListItem::createEditor(QWidget* parent, const QObject* receiver, const char* method) const
{
    QLineEdit *le = new QLineEdit(parent);
    le->setFrame(false);
    le->setReadOnly(true);
    return le;
}

void PropertyConstraintListItem::setEditorData(QWidget *editor, const QVariant& data) const
{
    QLineEdit* le = qobject_cast<QLineEdit*>(editor);
    le->setText(data.toString());
}

QVariant PropertyConstraintListItem::editorData(QWidget *editor) const
{
    QLineEdit *le = qobject_cast<QLineEdit*>(editor);
    return QVariant(le->text());
}

bool PropertyConstraintListItem::setQTProperty(const char* name, const QVariant & value)
{
    const Sketcher::PropertyConstraintList* item=static_cast<const Sketcher::PropertyConstraintList*>(getPropertyData()[0]);
    
    const std::vector< Sketcher::Constraint * > &vals = item->getValues();
    
    std::string sname(name);
    
    int i=0;
    for(std::vector< Sketcher::Constraint * >::const_iterator it= vals.begin();it!=vals.end();++it,++i){

        if((*it)->Name==sname){
            
            const Base::Quantity& val = value.value<Base::Quantity>();
            
            const_cast<Sketcher::Constraint *>((*it))->Value=val.getValue();
            
            const_cast<Sketcher::PropertyConstraintList*>(item)->set1Value(i,(*it));
            return false;
  
        }
    }
    
}

void PropertyConstraintListItem::initialize()
{
    
    const Sketcher::PropertyConstraintList* item=static_cast<const Sketcher::PropertyConstraintList*>(getPropertyData()[0]);
    
    const std::vector< Sketcher::Constraint * > &vals = item->getValues();
    
    for(std::vector< Sketcher::Constraint * >::const_iterator it= vals.begin();it!=vals.end();++it){
        if (!(*it)->Name.empty() && // Named constraint
            ((*it)->Type == Sketcher::Distance || // Datum constraint
            (*it)->Type == Sketcher::DistanceX ||
            (*it)->Type == Sketcher::DistanceY ||
            (*it)->Type == Sketcher::Radius ||
            (*it)->Type == Sketcher::Angle)) {
                    
            Base::Quantity pval;
            switch((*it)->Type){
                case Sketcher::Angle:
                    pval=Base::Quantity((*it)->Value, Base::Unit::Angle);
                    break;
                default:
                    pval=Base::Quantity((*it)->Value, Base::Unit::Length);
                    break;
            }
            
            
                PropertyUnitItem* mp=static_cast<PropertyUnitItem *>(PropertyUnitItem::create());
                mp->setParent(this);
                mp->setPropertyName(QString::fromUtf8((*it)->Name.c_str()));
                
                this->appendChild(mp);
                //this->setProperty((*it)->Name.c_str(),QVariant::fromValue<Base::Quantity>(pval));
            
        }

    }
    
     
    
}

void PropertyConstraintListItem::fillInSubProperties(const App::Property* prop, QString &valuestr) const
{   
    
    const std::vector< Sketcher::Constraint * > &vals = static_cast<const Sketcher::PropertyConstraintList*>(prop)->getValues();

    valuestr=QString::fromAscii("[");
    
    int i=0;
    for(std::vector< Sketcher::Constraint * >::const_iterator it= vals.begin();it!=vals.end();++it){
        if (!(*it)->Name.empty() && // Named constraint
            ((*it)->Type == Sketcher::Distance || // Datum constraint
            (*it)->Type == Sketcher::DistanceX ||
            (*it)->Type == Sketcher::DistanceY ||
            (*it)->Type == Sketcher::Radius ||
            (*it)->Type == Sketcher::Angle)) {
            i++;
        
            Base::Quantity pval;
            switch((*it)->Type){
                case Sketcher::Angle:
                    pval=Base::Quantity((*it)->Value, Base::Unit::Angle);
                    valuestr+= i==1? 
                        pval.getUserString():
                        QString::fromAscii("  ") +
                        pval.getUserString();
                    break;
                default:
                    pval=Base::Quantity((*it)->Value, Base::Unit::Length);
                    valuestr+= i==1?
                        pval.getUserString():
                        QString::fromAscii("  ") +
                        pval.getUserString();
                    break;
            }
            
            const_cast<PropertyConstraintListItem *>(this)->setProperty((*it)->Name.c_str(),QVariant::fromValue<Base::Quantity>(pval));
        }

    }
     
    valuestr+=QString::fromAscii("]");
    
}

#include "moc_PropertyConstraintListItem.cpp"