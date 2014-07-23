/***************************************************************************
 *   Copyright (c) 2014 Abdullah Tahiri <abdullah.tahiri.yo@gmail.com	     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "PreCompiled.h"
#ifndef _PreComp_
# include <cfloat>
# include <QMessageBox>
# include <Precision.hxx>
#endif

#include <App/Application.h>
#include <Gui/Application.h>
#include <Gui/Document.h>
#include <Gui/Selection.h>
#include <Gui/Command.h>
#include <Gui/MainWindow.h>
#include <Gui/DlgEditFileIncludeProptertyExternal.h>

#include <Mod/Part/App/Geometry.h>
#include <Mod/Sketcher/App/SketchObject.h>

#include "ViewProviderSketch.h"

using namespace std;
using namespace SketcherGui;
using namespace Sketcher;

bool isConstraintAcceleratorActive(Gui::Document *doc)
{
    if (doc)
        // checks if a Sketch Viewprovider is in Edit and is in no special mode
        if (doc->getInEdit() && doc->getInEdit()->isDerivedFrom(SketcherGui::ViewProviderSketch::getClassTypeId()))
            if (dynamic_cast<SketcherGui::ViewProviderSketch*>(doc->getInEdit())
                ->getSketchMode() == ViewProviderSketch::STATUS_NONE)
                if (Gui::Selection().countObjectsOfType(Sketcher::SketchObject::getClassTypeId()) > 0)
                    return true;
    return false;
}

// Close Shape Command
DEF_STD_CMD_A(CmdSketcherCloseShape);

CmdSketcherCloseShape::CmdSketcherCloseShape()
    :Command("Sketcher_CloseShape")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Close Shape");
    sToolTipText    = QT_TR_NOOP("Produce closed shape by Link end point of element with next elements' starting point");
    sWhatsThis      = sToolTipText;
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_CloseShape";
    sAccel          = "N";
    eType           = ForEdit;
}

void CmdSketcherCloseShape::activated(int iMsg)
{
    // get the selection
    std::vector<Gui::SelectionObject> selection = getSelection().getSelectionEx();

    // only one sketch with its subelements are allowed to be selected
    if (selection.size() != 1) {
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Wrong selection"),
            QObject::tr("Select an edge from the sketch."));
        return;
    }

    // get the needed lists and objects
    const std::vector<std::string> &SubNames = selection[0].getSubNames();
    Sketcher::SketchObject* Obj = dynamic_cast<Sketcher::SketchObject*>(selection[0].getObject());

    int GeoIdFirst=-1;
    int GeoIdLast=-1;
    
    // go through the selected subelements
    for (unsigned int i=0; i<(SubNames.size()-1); i++ ) {
        // only handle edges
        if (SubNames[i].size() > 4 && SubNames[i].substr(0,4) == "Edge" &&
	    SubNames[i+1].size() > 4 && SubNames[i+1].substr(0,4) == "Edge"	) {

	    int GeoId1 = std::atoi(SubNames[i].substr(4,4000).c_str()) - 1;
	    int GeoId2 = std::atoi(SubNames[i+1].substr(4,4000).c_str()) - 1;
	
	    if(GeoIdFirst==-1)
	      GeoIdFirst=GeoId1;
	    
	    GeoIdLast=GeoId2;

	    const Part::Geometry *geo1 = Obj->getGeometry(GeoId1);
	    const Part::Geometry *geo2 = Obj->getGeometry(GeoId2);
	    if (	(geo1->getTypeId() != Part::GeomLineSegment::getClassTypeId() &&
		geo1->getTypeId() != Part::GeomArcOfCircle::getClassTypeId()	) ||
		(geo2->getTypeId() != Part::GeomLineSegment::getClassTypeId() &&
		geo2->getTypeId() != Part::GeomArcOfCircle::getClassTypeId())	) {
		QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Impossible constraint"),
				      QObject::tr("One selected edge is not connectable"));
		return;
	    }
            
	    // undo command open
	    openCommand("add coincident constraint");
	    Gui::Command::doCommand(
		Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Coincident',%d,%d,%d,%d)) ",
		selection[0].getFeatName(),GeoId1,Sketcher::end,GeoId2,Sketcher::start);

	    // finish the transaction and update
	    commitCommand();
	    

        }
    }
    
    // Close Last Edge with First Edge
    // undo command open
    openCommand("add coincident constraint");
    Gui::Command::doCommand(
	Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Coincident',%d,%d,%d,%d)) ",
	selection[0].getFeatName(),GeoIdLast,Sketcher::end,GeoIdFirst,Sketcher::start);    
    
    // finish the transaction and update
    commitCommand();

    updateActive();

    // clear the selection (convenience)
    getSelection().clearSelection();
}

bool CmdSketcherCloseShape::isActive(void)
{
    return isConstraintAcceleratorActive( getActiveGuiDocument() );
}


// Connect Edges Command
DEF_STD_CMD_A(CmdSketcherConnect);

CmdSketcherConnect::CmdSketcherConnect()
    :Command("Sketcher_ConnectLines")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Connect Edges");
    sToolTipText    = QT_TR_NOOP("Link end point of element with next elements' starting point");
    sWhatsThis      = sToolTipText;
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_CloseShape";
    sAccel          = "N";
    eType           = ForEdit;
}

void CmdSketcherConnect::activated(int iMsg)
{
    // get the selection
    std::vector<Gui::SelectionObject> selection = getSelection().getSelectionEx();

    // only one sketch with its subelements are allowed to be selected
    if (selection.size() != 1) {
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Wrong selection"),
            QObject::tr("Select an edge from the sketch."));
        return;
    }

    // get the needed lists and objects
    const std::vector<std::string> &SubNames = selection[0].getSubNames();
    Sketcher::SketchObject* Obj = dynamic_cast<Sketcher::SketchObject*>(selection[0].getObject());
    
    // go through the selected subelements
    for (unsigned int i=0; i<(SubNames.size()-1); i++ ) {
        // only handle edges
        if (SubNames[i].size() > 4 && SubNames[i].substr(0,4) == "Edge" &&
	    SubNames[i+1].size() > 4 && SubNames[i+1].substr(0,4) == "Edge"	) {

	    int GeoId1 = std::atoi(SubNames[i].substr(4,4000).c_str()) - 1;
	    int GeoId2 = std::atoi(SubNames[i+1].substr(4,4000).c_str()) - 1;

	    const Part::Geometry *geo1 = Obj->getGeometry(GeoId1);
	    const Part::Geometry *geo2 = Obj->getGeometry(GeoId2);
	    if (	(geo1->getTypeId() != Part::GeomLineSegment::getClassTypeId() &&
		geo1->getTypeId() != Part::GeomArcOfCircle::getClassTypeId()	) ||
		(geo2->getTypeId() != Part::GeomLineSegment::getClassTypeId() &&
		geo2->getTypeId() != Part::GeomArcOfCircle::getClassTypeId())	) {
		QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Impossible constraint"),
				      QObject::tr("One selected edge is not connectable"));
		return;
	    }
            
	    // undo command open
	    openCommand("add coincident constraint");
	    Gui::Command::doCommand(
		Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Coincident',%d,%d,%d,%d)) ",
		selection[0].getFeatName(),GeoId1,Sketcher::end,GeoId2,Sketcher::start);

	    // finish the transaction and update
	    commitCommand();
	    

        }
    }

    updateActive();

    // clear the selection (convenience)
    getSelection().clearSelection();
}

bool CmdSketcherConnect::isActive(void)
{
    return isConstraintAcceleratorActive( getActiveGuiDocument() );
}

// Select Constraints of selected elements
DEF_STD_CMD_A(CmdSketcherSelectConstraints);

CmdSketcherSelectConstraints::CmdSketcherSelectConstraints()
    :Command("Sketcher_SelectConstraints")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Select Constraints");
    sToolTipText    = QT_TR_NOOP("Select the constraints associated to the selected elements");
    sWhatsThis      = sToolTipText;
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_SelectConstraints";
    sAccel          = "M";
    eType           = ForEdit;
}

void CmdSketcherSelectConstraints::activated(int iMsg)
{
    // get the selection
    std::vector<Gui::SelectionObject> selection = getSelection().getSelectionEx();
    Sketcher::SketchObject* Obj = dynamic_cast<Sketcher::SketchObject*>(selection[0].getObject());

    // only one sketch with its subelements are allowed to be selected
    if (selection.size() != 1) {
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Wrong selection"),
            QObject::tr("Select elements from a single sketch."));
        return;
    }

    // get the needed lists and objects
    const std::vector<std::string> &SubNames = selection[0].getSubNames();
    const std::vector< Sketcher::Constraint * > &vals = Obj->Constraints.getValues();
    
    std::string doc_name = Obj->getDocument()->getName();
    std::string obj_name = Obj->getNameInDocument();
    std::stringstream ss;
    
    getSelection().clearSelection();
    
    // go through the selected subelements
    for (std::vector<std::string>::const_iterator it=SubNames.begin(); it != SubNames.end(); ++it) {
        // only handle edges
        if (it->size() > 4 && it->substr(0,4) == "Edge") {
            int GeoId = std::atoi(it->substr(4,4000).c_str()) - 1;
            
            // push all the constraints
	    int i=1;
            for (std::vector< Sketcher::Constraint * >::const_iterator it= vals.begin();
                 it != vals.end(); ++it,++i) {
                if ( (*it)->First == GeoId || (*it)->Second == GeoId || (*it)->Third == GeoId){
		  ss.str(std::string());
		  ss << "Constraint" << i;
		  Gui::Selection().addSelection(doc_name.c_str(), obj_name.c_str(), ss.str().c_str());
		}
            }
        }
    }

}

bool CmdSketcherSelectConstraints::isActive(void)
{
    return isConstraintAcceleratorActive( getActiveGuiDocument() );
}


// Add Accelerator Commands
void CreateSketcherCommandsConstraintAccel(void)
{
    Gui::CommandManager &rcCmdMgr = Gui::Application::Instance->commandManager();

    rcCmdMgr.addCommand(new CmdSketcherCloseShape());
    rcCmdMgr.addCommand(new CmdSketcherConnect());
    rcCmdMgr.addCommand(new CmdSketcherSelectConstraints());
}
