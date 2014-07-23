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
# include <QRegExp>
# include <QString>
#endif

#include "TaskSketcherElements.h"
#include "ui_TaskSketcherElements.h"
#include "EditDatumDialog.h"
#include "ViewProviderSketch.h"

#include <Mod/Sketcher/App/SketchObject.h>

#include <Base/Tools.h>
#include <App/Application.h>
#include <App/Document.h>
#include <Gui/Application.h>
#include <Gui/Document.h>
#include <Gui/Selection.h>
#include <Gui/BitmapFactory.h>
#include <Gui/ViewProvider.h>
#include <Gui/BitmapFactory.h>
#include <boost/bind.hpp>
#include <Gui/Command.h>

using namespace SketcherGui;
using namespace Gui::TaskView;

#define CONTEXT_ITEM(ICONSTR,NAMESTR,FUNC,KEY) 						\
QIcon icon_ ## FUNC( Gui::BitmapFactory().pixmap(ICONSTR) ); 					\
    QAction* constr_ ## FUNC = menu.addAction(icon_ ## FUNC,tr(NAMESTR), this, SLOT(FUNC()), 	\
        QKeySequence(KEY));								\
    constr_ ## FUNC->setEnabled(!items.isEmpty());

#define CONTEXT_MEMBER_DEF(CMDSTR,FUNC) 							\
void ElementView::FUNC(){								\
   Gui::Application::Instance->commandManager().runCommandByName(CMDSTR);}

#define CONTEXT_SHORTCUT(KEY,FUNC) 							\
         case KEY : FUNC();break;
	 
// helper class to store additional information about the listWidget entry.
class ElementItem : public QListWidgetItem
{
public:
    ElementItem(	const QIcon & icon, const QString & text, int elementnr, 
		int startingVertex, int midVertex, int endVertex,
		Base::Type geometryType)
        : QListWidgetItem(icon,text),ElementNbr(elementnr),
	  StartingVertex(startingVertex), MidVertex(midVertex), EndVertex(endVertex),
	  GeometryType(geometryType),isLineSelected(false),
	  isStartingPointSelected(false),isEndPointSelected(false),isMidPointSelected(false)
    {
          
    }
    ElementItem( const QString & text,int elementnr,
		int startingVertex, int midVertex, int endVertex,
		Base::Type geometryType)
        : QListWidgetItem(text),ElementNbr(elementnr),
	  StartingVertex(startingVertex), MidVertex(midVertex), EndVertex(endVertex),
	  GeometryType(geometryType),isLineSelected(false),
	  isStartingPointSelected(false),isEndPointSelected(false),isMidPointSelected(false)
    {
      
    }
    ~ElementItem()
    {
    }

    int ElementNbr;
    int StartingVertex;
    int MidVertex;
    int EndVertex;
    bool isLineSelected;
    bool isStartingPointSelected;
    bool isEndPointSelected;
    bool isMidPointSelected;
    Base::Type GeometryType;
};

ElementView::ElementView(QWidget *parent)
    : QListWidget(parent)
{
}

ElementView::~ElementView()
{
}

void ElementView::contextMenuEvent (QContextMenuEvent* event)
{
    QMenu menu;
    QListWidgetItem* item = currentItem();
    QList<QListWidgetItem *> items = selectedItems();

    // CONTEXT_ITEM(ICONSTR,NAMESTR,FUNC,KEY)
    CONTEXT_ITEM("Constraint_PointOnPoint","Point Coincidence",doPointCoincidence,Qt::Key_C)
    CONTEXT_ITEM("Constraint_PointOnObject","Point on Object",doPointOnObjectConstraint,Qt::Key_Q)
    CONTEXT_ITEM("Constraint_Vertical","Vertical Constraint",doVerticalConstraint,Qt::Key_V)
    CONTEXT_ITEM("Constraint_Horizontal","Horizontal Constraint",doHorizontalConstraint,Qt::Key_H)
    CONTEXT_ITEM("Constraint_Parallel","Parallel Constraint",doParallelConstraint,Qt::Key_Y)
    CONTEXT_ITEM("Constraint_Perpendicular","Perpendicular Constraint",doPerpendicularConstraint,Qt::Key_G)
    CONTEXT_ITEM("Constraint_Tangent","Tangent Constraint",doTangentConstraint,Qt::Key_W)
    CONTEXT_ITEM("Constraint_EqualLength","Equal Length",doEqualConstraint,Qt::Key_J)
    CONTEXT_ITEM("Constraint_Symmetric","Symetric",doSymetricConstraint,Qt::Key_S)
    CONTEXT_ITEM("Sketcher_ConstrainLock","Lock Constraint",doLockConstraint,Qt::Key_B)    
    CONTEXT_ITEM("Constraint_HorizontalDistance","Horizontal Distance",doHorizontalDistance,Qt::Key_K)
    CONTEXT_ITEM("Constraint_VerticalDistance","Vertical Distance",doVerticalDistance,Qt::Key_I)
    CONTEXT_ITEM("Constraint_Length","Length Constraint",doLengthConstraint,Qt::Key_Z)
    CONTEXT_ITEM("Constraint_Radius","Radius Constraint",doRadiusConstraint,Qt::Key_X)
    CONTEXT_ITEM("Constraint_InternalAngle","Angle Constraint",doAngleConstraint,Qt::Key_A)
    
    QAction* sep = menu.addSeparator();
    
    CONTEXT_ITEM("Sketcher_AlterConstruction","Toggle construction line",doToggleConstruction,Qt::Key_T)
    
    QAction* sep1 = menu.addSeparator();
    
    CONTEXT_ITEM("Sketcher_CloseShape","Close Shape",doCloseShape,Qt::Key_N)
    CONTEXT_ITEM("Sketcher_ConnectLines","Connect",doConnect,Qt::Key_M)
    CONTEXT_ITEM("Sketcher_SelectConstraints","Select Constraints",doSelectConstraints,Qt::Key_M)
    
    QAction* sep2 = menu.addSeparator();
        
    QAction* remove = menu.addAction(tr("Delete"), this, SLOT(deleteSelectedItems()),
        QKeySequence(QKeySequence::Delete));
    remove->setEnabled(!items.isEmpty());

    menu.menuAction()->setIconVisibleInMenu(true);
    
    menu.exec(event->globalPos());
}

CONTEXT_MEMBER_DEF("Sketcher_ConstrainDistanceX",doHorizontalDistance)
CONTEXT_MEMBER_DEF("Sketcher_ConstrainDistanceY",doVerticalDistance)
CONTEXT_MEMBER_DEF("Sketcher_ConstrainHorizontal",doHorizontalConstraint)
CONTEXT_MEMBER_DEF("Sketcher_ConstrainVertical",doVerticalConstraint)
CONTEXT_MEMBER_DEF("Sketcher_ConstrainLock",doLockConstraint)
CONTEXT_MEMBER_DEF("Sketcher_ConstrainCoincident",doPointCoincidence)
CONTEXT_MEMBER_DEF("Sketcher_ConstrainParallel",doParallelConstraint)
CONTEXT_MEMBER_DEF("Sketcher_ConstrainPerpendicular",doPerpendicularConstraint)
CONTEXT_MEMBER_DEF("Sketcher_ConstrainDistance",doLengthConstraint)
CONTEXT_MEMBER_DEF("Sketcher_ConstrainRadius",doRadiusConstraint)
CONTEXT_MEMBER_DEF("Sketcher_ConstrainAngle",doAngleConstraint)
CONTEXT_MEMBER_DEF("Sketcher_ConstrainEqual",doEqualConstraint)
CONTEXT_MEMBER_DEF("Sketcher_ConstrainPointOnObject",doPointOnObjectConstraint)
CONTEXT_MEMBER_DEF("Sketcher_ConstrainSymmetric",doSymetricConstraint)
CONTEXT_MEMBER_DEF("Sketcher_ConstrainTangent",doTangentConstraint)
CONTEXT_MEMBER_DEF("Sketcher_CloseShape",doCloseShape)
CONTEXT_MEMBER_DEF("Sketcher_ConnectLines",doConnect)
CONTEXT_MEMBER_DEF("Sketcher_ToggleConstruction",doToggleConstruction)
CONTEXT_MEMBER_DEF("Sketcher_SelectConstraints",doSelectConstraints)



void ElementView::deleteSelectedItems()
{
    App::Document* doc = App::GetApplication().getActiveDocument();
    if (!doc) return;

    doc->openTransaction("Delete");
    std::vector<Gui::SelectionObject> sel = Gui::Selection().getSelectionEx(doc->getName());
    for (std::vector<Gui::SelectionObject>::iterator ft = sel.begin(); ft != sel.end(); ++ft) {
        Gui::ViewProvider* vp = Gui::Application::Instance->getViewProvider(ft->getObject());
        if (vp) {
            vp->onDelete(ft->getSubNames());
        }
    }
    doc->commitTransaction();
}


void ElementView::keyPressEvent(QKeyEvent * event)
{
    switch (event->key())
    {
      case Qt::Key_Shift:
	// signal
	onFilterChange();
	break;
      CONTEXT_SHORTCUT(Qt::Key_C,doPointCoincidence)
      CONTEXT_SHORTCUT(Qt::Key_Q,doPointOnObjectConstraint)
      CONTEXT_SHORTCUT(Qt::Key_V,doVerticalConstraint)
      CONTEXT_SHORTCUT(Qt::Key_H,doHorizontalConstraint)
      CONTEXT_SHORTCUT(Qt::Key_Y,doParallelConstraint)
      CONTEXT_SHORTCUT(Qt::Key_G,doPerpendicularConstraint)
      CONTEXT_SHORTCUT(Qt::Key_W,doTangentConstraint)
      CONTEXT_SHORTCUT(Qt::Key_J,doEqualConstraint)
      CONTEXT_SHORTCUT(Qt::Key_S,doSymetricConstraint)
      CONTEXT_SHORTCUT(Qt::Key_B,doLockConstraint)
      CONTEXT_SHORTCUT(Qt::Key_K,doHorizontalDistance)
      CONTEXT_SHORTCUT(Qt::Key_I,doVerticalDistance)
      CONTEXT_SHORTCUT(Qt::Key_Z,doLengthConstraint)
      CONTEXT_SHORTCUT(Qt::Key_X,doRadiusConstraint)
      CONTEXT_SHORTCUT(Qt::Key_A,doAngleConstraint)
      CONTEXT_SHORTCUT(Qt::Key_N,doCloseShape)
      default:
	QListWidget::keyPressEvent( event );
	break;
    }
}
// ----------------------------------------------------------------------------

TaskSketcherElements::TaskSketcherElements(ViewProviderSketch *sketchView)
    : TaskBox(Gui::BitmapFactory().pixmap("document-new"),tr("Elements"),true, 0),
    sketchView(sketchView), inhibitSelectionUpdate(false), focusItemIndex(-1)
{
    // we need a separate container widget to add all controls to
    proxy = new QWidget(this);
    ui = new Ui_TaskSketcherElements();
    ui->setupUi(proxy);
    ui->listWidgetElements->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->listWidgetElements->setEditTriggers(QListWidget::NoEditTriggers);
    ui->listWidgetElements->setMouseTracking(true);

    // connecting the needed signals
    QObject::connect(
        ui->listWidgetElements, SIGNAL(itemSelectionChanged()),
        this                     , SLOT  (on_listWidgetElements_itemSelectionChanged())
       );
    QObject::connect(
        ui->listWidgetElements, SIGNAL(itemEntered(QListWidgetItem *)),
        this                     , SLOT  (on_listWidgetElements_itemEntered(QListWidgetItem *))
       );
    QObject::connect(
        ui->listWidgetElements, SIGNAL(onFilterChange()),
        this                     , SLOT  (on_listWidgetElements_shiftPressed())
       );
    QObject::connect(
        ui->comboBoxElementFilter, SIGNAL(currentIndexChanged(int)),
        this                     , SLOT  (on_listWidgetElements_currentFilterChanged(int))
       );
    
    connectionElementsChanged = sketchView->signalElementsChanged.connect(
        boost::bind(&SketcherGui::TaskSketcherElements::slotElementsChanged, this));
    
    this->groupLayout()->addWidget(proxy);
    
    ui->comboBoxElementFilter->setCurrentIndex(0);

    slotElementsChanged();
}

TaskSketcherElements::~TaskSketcherElements()
{
    connectionElementsChanged.disconnect();
    delete ui;
}

void TaskSketcherElements::onSelectionChanged(const Gui::SelectionChanges& msg)
{
    std::string temp;
    if (msg.Type == Gui::SelectionChanges::ClrSelection) {
	clearWidget();
    }
    else if (msg.Type == Gui::SelectionChanges::AddSelection ||
             msg.Type == Gui::SelectionChanges::RmvSelection) {
        bool select = (msg.Type == Gui::SelectionChanges::AddSelection);
        // is it this object??
        if (strcmp(msg.pDocName,sketchView->getSketchObject()->getDocument()->getName())==0 &&
            strcmp(msg.pObjectName,sketchView->getSketchObject()->getNameInDocument())== 0) {
            if (msg.pSubName) {
                QString expr = QString::fromAscii(msg.pSubName);
		std::string shapetype(msg.pSubName);
		// if-else edge vertex
		if (shapetype.size() > 4 && shapetype.substr(0,4) == "Edge")
		{
		  QRegExp rx(QString::fromAscii("^Edge(\\d+)$"));
		  int pos = expr.indexOf(rx);
		  if (pos > -1) {
		      bool ok;
		      int ElementId = rx.cap(1).toInt(&ok) - 1;
		      if (ok) {
			  int countItems = ui->listWidgetElements->count();
			  for (int i=0; i < countItems; i++) {
			      ElementItem* item = static_cast<ElementItem*>
				  (ui->listWidgetElements->item(i));
			      if (item->ElementNbr == ElementId) {
				  item->isLineSelected=select;
				  break;
			      }
			  }
		      }
		  }
		}
		else if (shapetype.size() > 6 && shapetype.substr(0,6) == "Vertex"){
		  QRegExp rx(QString::fromAscii("^Vertex(\\d+)$"));
		  int pos = expr.indexOf(rx);
		  if (pos > -1) {
		      bool ok;
		      int ElementId = rx.cap(1).toInt(&ok) - 1;
		      if (ok) {
			  // Get the GeoID&Pos
			  int GeoId; 
			  Sketcher::PointPos PosId;
			  sketchView->getSketchObject()->getGeoVertexIndex(ElementId,GeoId, PosId);
			  
			  int countItems = ui->listWidgetElements->count();
			  for (int i=0; i < countItems; i++) {
			      ElementItem* item = static_cast<ElementItem*>
				  (ui->listWidgetElements->item(i));
			      if (item->ElementNbr == GeoId) {
				  switch(PosId)
				  {
				    case Sketcher::start:
				    item->isStartingPointSelected=select;
				    break;
				    case Sketcher::end:
				    item->isEndPointSelected=select;
				    break;
				    case Sketcher::mid:
				    item->isMidPointSelected=select;
				    break;				    
				  }
				  break;
			      }
			  }
		      }
		  }		  		  
		}
		// update the listwidget
		int element=ui->comboBoxElementFilter->currentIndex();

		ui->listWidgetElements->blockSignals(true);


		for (int i=0;i<ui->listWidgetElements->count(); i++) {
		    ElementItem * ite=static_cast<ElementItem*>(ui->listWidgetElements->item(i));
		       
		    switch(element){
		      case 0:		
			  ite->setSelected(ite->isLineSelected);
			  break;
		      case 1:
			  ite->setSelected(ite->isStartingPointSelected);
			  break;
		      case 2:
			  ite->setSelected(ite->isEndPointSelected);
			  break;
		      case 3:
			  ite->setSelected(ite->isMidPointSelected);
			  break;	
		    }
		}

		ui->listWidgetElements->blockSignals(false);

            }
        }
    }
    else if (msg.Type == Gui::SelectionChanges::SetSelection) {
        // do nothing here
    }
}


void TaskSketcherElements::on_listWidgetElements_itemSelectionChanged(void)
{
    ui->listWidgetElements->blockSignals(true);

      
    // selection changed because we acted on the current entered item
    // we can not do this with ItemPressed because that signal is triggered after this one
    int element=ui->comboBoxElementFilter->currentIndex();
    
    ElementItem * itf;
    
    if(focusItemIndex>-1 && focusItemIndex<ui->listWidgetElements->count())
      itf=static_cast<ElementItem*>(ui->listWidgetElements->item(focusItemIndex));
    else
      itf=NULL;
    
    bool multipleselection=true;
    
    if(!inhibitSelectionUpdate){	
      if(itf!=NULL) {
	switch(element){
	  case 0:
	      itf->isLineSelected=!itf->isLineSelected;	
	      break;
	  case 1:
	      itf->isStartingPointSelected=!itf->isStartingPointSelected;
	      break;
	  case 2:
	      itf->isEndPointSelected=!itf->isEndPointSelected;
	      break;
	  case 3:
	      itf->isMidPointSelected=!itf->isMidPointSelected;
	      break;	
	} 
      }
      
      if(QApplication::keyboardModifiers()==Qt::ControlModifier)// multiple selection?
	multipleselection=true;
      else
	multipleselection=false;
    }      
    
    std::string doc_name = sketchView->getSketchObject()->getDocument()->getName();
    std::string obj_name = sketchView->getSketchObject()->getNameInDocument();

    bool block = this->blockConnection(true); // avoid to be notified by itself
    Gui::Selection().clearSelection();
	  
    for (int i=0;i<ui->listWidgetElements->count(); i++) {
	ElementItem * ite=static_cast<ElementItem*>(ui->listWidgetElements->item(i));
	
	if(multipleselection==false && ite!=itf)
	{
	
	  ite->isLineSelected=false;
	  ite->isStartingPointSelected=false;
	  ite->isEndPointSelected=false;
	  ite->isMidPointSelected=false;
	  
	}
	
	// first update the listwidget
	switch(element){
	  case 0:		
	      ite->setSelected(ite->isLineSelected);
	      break;
	  case 1:
	      ite->setSelected(ite->isStartingPointSelected);
	      break;
	  case 2:
	      ite->setSelected(ite->isEndPointSelected);
	      break;
	  case 3:
	      ite->setSelected(ite->isMidPointSelected);
	      break;	
	}
	
	// now the scene
	std::stringstream ss;
	int vertex;
	
	if(ite->isLineSelected){
	  ss << "Edge" << ite->ElementNbr + 1;
	  Gui::Selection().addSelection(doc_name.c_str(), obj_name.c_str(), ss.str().c_str());
	}
	
	if(ite->isStartingPointSelected){
	  ss.str(std::string());
	  vertex= ite->StartingVertex;
	  if(vertex!=-1){
	    ss << "Vertex" << vertex + 1;
	    Gui::Selection().addSelection(doc_name.c_str(), obj_name.c_str(), ss.str().c_str());
	  }	  
	}
	
	if(ite->isEndPointSelected){
	  ss.str(std::string());
	  vertex= ite->EndVertex;
	  if(vertex!=-1){
	    ss << "Vertex" << vertex + 1;
	    Gui::Selection().addSelection(doc_name.c_str(), obj_name.c_str(), ss.str().c_str());
	  }	  
	}
	
	if(ite->isMidPointSelected){
	  ss.str(std::string());
	  vertex= ite->MidVertex;
	  if(vertex!=-1){
	    ss << "Vertex" << vertex + 1;
	    Gui::Selection().addSelection(doc_name.c_str(), obj_name.c_str(), ss.str().c_str());
	  }	  
	}
    }
    this->blockConnection(block);
    ui->listWidgetElements->blockSignals(false);
}

void TaskSketcherElements::on_listWidgetElements_itemEntered(QListWidgetItem *item)
{
    ElementItem *it = dynamic_cast<ElementItem*>(item);
    if (!item) return;
    
    Gui::Selection().rmvPreselect();
    
    ui->listWidgetElements->setFocus();
    
    focusItemIndex=ui->listWidgetElements->row(item);
    
    std::string doc_name = sketchView->getSketchObject()->getDocument()->getName();
    std::string obj_name = sketchView->getSketchObject()->getNameInDocument();

    /* 0 - Lines
     * 1 - Starting Points
     * 2 - End Points
     * 3 - Middle Points
     */
    std::stringstream ss;
    
    int element=ui->comboBoxElementFilter->currentIndex();
    int vertex;
    
    switch(element)
    {
      case 0: 
	ss << "Edge" << it->ElementNbr + 1;
	Gui::Selection().setPreselect(doc_name.c_str(), obj_name.c_str(), ss.str().c_str());
	break;
      case 1:
      case 2:
      case 3:	
	vertex= sketchView->getSketchObject()->getVertexIndexGeoPos(it->ElementNbr,static_cast<Sketcher::PointPos>(element));
	if(vertex!=-1){
	  ss << "Vertex" << vertex + 1;
	  Gui::Selection().setPreselect(doc_name.c_str(), obj_name.c_str(), ss.str().c_str());
	}
	break;
    }
    
}

void TaskSketcherElements::leaveEvent ( QEvent * event )
{
  Gui::Selection().rmvPreselect();
  ui->listWidgetElements->clearFocus();
  //Base::leaveEvent(event);
}

void TaskSketcherElements::slotElementsChanged(void)
{ 
    QIcon Sketcher_Element_Arc_Edge( Gui::BitmapFactory().pixmap("Sketcher_Element_Arc_Edge") );
    QIcon Sketcher_Element_Arc_EndPoint( Gui::BitmapFactory().pixmap("Sketcher_Element_Arc_EndPoint") );
    QIcon Sketcher_Element_Arc_MidPoint( Gui::BitmapFactory().pixmap("Sketcher_Element_Arc_MidPoint") );
    QIcon Sketcher_Element_Arc_StartingPoint( Gui::BitmapFactory().pixmap("Sketcher_Element_Arc_StartingPoint") );
    QIcon Sketcher_Element_Circle_Edge( Gui::BitmapFactory().pixmap("Sketcher_Element_Circle_Edge") );
    QIcon Sketcher_Element_Circle_MidPoint( Gui::BitmapFactory().pixmap("Sketcher_Element_Circle_MidPoint") );
    QIcon Sketcher_Element_Line_Edge( Gui::BitmapFactory().pixmap("Sketcher_Element_Line_Edge") );
    QIcon Sketcher_Element_Line_EndPoint( Gui::BitmapFactory().pixmap("Sketcher_Element_Line_EndPoint") );
    QIcon Sketcher_Element_Line_StartingPoint( Gui::BitmapFactory().pixmap("Sketcher_Element_Line_StartingPoint") );
    QIcon Sketcher_Element_Point_StartingPoint( Gui::BitmapFactory().pixmap("Sketcher_Element_Point_StartingPoint") );

    QIcon none( Gui::BitmapFactory().pixmap("Sketcher_ConstrainLock") );

    assert(sketchView);
    // Build up ListView with the elements
    const std::vector< Part::Geometry * > &vals = sketchView->getSketchObject()->Geometry.getValues();
    
    ui->listWidgetElements->clear();

    int element = ui->comboBoxElementFilter->currentIndex();
    
    int i=1;
    for(std::vector< Part::Geometry * >::const_iterator it= vals.begin();it!=vals.end();++it,++i){
      Base::Type type = (*it)->getTypeId();      
      
      ui->listWidgetElements->addItem(new ElementItem(
	(type == Part::GeomPoint::getClassTypeId() 	&& element==1) ? Sketcher_Element_Point_StartingPoint :
	(type == Part::GeomLineSegment::getClassTypeId()  && element==0) ? Sketcher_Element_Line_Edge :
	(type == Part::GeomLineSegment::getClassTypeId()  && element==1) ? Sketcher_Element_Line_StartingPoint :
	(type == Part::GeomLineSegment::getClassTypeId()  && element==2) ? Sketcher_Element_Line_EndPoint :
	(type == Part::GeomArcOfCircle::getClassTypeId() 	&& element==0) ? Sketcher_Element_Arc_Edge :
	(type == Part::GeomArcOfCircle::getClassTypeId() 	&& element==1) ? Sketcher_Element_Arc_StartingPoint :
	(type == Part::GeomArcOfCircle::getClassTypeId() 	&& element==2) ? Sketcher_Element_Arc_EndPoint :
	(type == Part::GeomArcOfCircle::getClassTypeId() 	&& element==3) ? Sketcher_Element_Arc_MidPoint :
	(type == Part::GeomCircle::getClassTypeId()	&& element==0) ? Sketcher_Element_Circle_Edge :
	(type == Part::GeomCircle::getClassTypeId()	&& element==3) ? Sketcher_Element_Circle_MidPoint :
	none,
	type == Part::GeomPoint::getClassTypeId() 	? tr("Point") + QString::fromLatin1("(Edge%1)").arg(i) :
	type == Part::GeomLineSegment::getClassTypeId()	? tr("Line") + QString::fromLatin1("(Edge%1)").arg(i) :
	type == Part::GeomArcOfCircle::getClassTypeId()	? tr("Arc") + QString::fromLatin1("(Edge%1)").arg(i) :
	type == Part::GeomCircle::getClassTypeId()	? tr("Circle") + QString::fromLatin1("(Edge%1)").arg(i) :
	tr("Other") + QString::fromLatin1("(Edge%1)").arg(i),
	i-1,
	sketchView->getSketchObject()->getVertexIndexGeoPos(i-1,Sketcher::start),
	sketchView->getSketchObject()->getVertexIndexGeoPos(i-1,Sketcher::mid),
	sketchView->getSketchObject()->getVertexIndexGeoPos(i-1,Sketcher::end),
	type));  
    }
}


void TaskSketcherElements::on_listWidgetElements_shiftPressed()
{
    int element;
    
    // calculate next element type on shift press according to entered/preselected element
    // This is the aka fast-forward functionality
    if(focusItemIndex>-1 && focusItemIndex<ui->listWidgetElements->count()){
      
      ElementItem * itf=static_cast<ElementItem*>(ui->listWidgetElements->item(focusItemIndex));
      
      Base::Type type = itf->GeometryType;
      
      element = ui->comboBoxElementFilter->currentIndex(); // currently selected type index
      
      switch(element)
      {
	case 0: // Edge
	  element =	( type == Part::GeomCircle::getClassTypeId() ) ? 3 : 1;
	  break;
	case 1: // StartingPoint
	  element =	( type == Part::GeomCircle::getClassTypeId() ) ? 3 : 
			( type == Part::GeomPoint::getClassTypeId()  ) ? 1 : 2;
	  break;
	case 2: // EndPoint
	  element =	( type == Part::GeomLineSegment::getClassTypeId() ) ? 0 :
			( type == Part::GeomPoint::getClassTypeId()  ) ? 1 : 3;
	  break;
	case 3: // MidPoint
	  element =	( type == Part::GeomPoint::getClassTypeId()  ) ? 1 : 0;
	  break;
	default:
	  element = 0;
      }
           
      ui->comboBoxElementFilter->setCurrentIndex(element);
      
      Gui::Selection().rmvPreselect();
      
      on_listWidgetElements_itemEntered(itf);
    }
    else{
      element = (ui->comboBoxElementFilter->currentIndex()+1) % 
		ui->comboBoxElementFilter->count();
		
      ui->comboBoxElementFilter->setCurrentIndex(element);
      
      Gui::Selection().rmvPreselect();
    }
    
    //update the icon
    updateIcons(element);
    
    updatePreselection();
}


void TaskSketcherElements::on_listWidgetElements_currentFilterChanged ( int index )
{
    Gui::Selection().rmvPreselect();
    
    updateIcons(index);
  
    updatePreselection();
  
}    

void TaskSketcherElements::updatePreselection()
{
    inhibitSelectionUpdate=true;
    on_listWidgetElements_itemSelectionChanged();
    inhibitSelectionUpdate=false;
}

void TaskSketcherElements::clearWidget()
{
    ui->listWidgetElements->blockSignals(true);
    ui->listWidgetElements->clearSelection ();
    ui->listWidgetElements->blockSignals(false);
    
    // update widget
    int countItems = ui->listWidgetElements->count();
    for (int i=0; i < countItems; i++) {
      ElementItem* item = static_cast<ElementItem*> (ui->listWidgetElements->item(i));
      item->isLineSelected=false;
      item->isStartingPointSelected=false;
      item->isEndPointSelected=false;
      item->isMidPointSelected=false;		
    }
}

void TaskSketcherElements::updateIcons(int element)
{
    QIcon Sketcher_Element_Arc_Edge( Gui::BitmapFactory().pixmap("Sketcher_Element_Arc_Edge") );
    QIcon Sketcher_Element_Arc_EndPoint( Gui::BitmapFactory().pixmap("Sketcher_Element_Arc_EndPoint") );
    QIcon Sketcher_Element_Arc_MidPoint( Gui::BitmapFactory().pixmap("Sketcher_Element_Arc_MidPoint") );
    QIcon Sketcher_Element_Arc_StartingPoint( Gui::BitmapFactory().pixmap("Sketcher_Element_Arc_StartingPoint") );
    QIcon Sketcher_Element_Circle_Edge( Gui::BitmapFactory().pixmap("Sketcher_Element_Circle_Edge") );
    QIcon Sketcher_Element_Circle_MidPoint( Gui::BitmapFactory().pixmap("Sketcher_Element_Circle_MidPoint") );
    QIcon Sketcher_Element_Line_Edge( Gui::BitmapFactory().pixmap("Sketcher_Element_Line_Edge") );
    QIcon Sketcher_Element_Line_EndPoint( Gui::BitmapFactory().pixmap("Sketcher_Element_Line_EndPoint") );
    QIcon Sketcher_Element_Line_StartingPoint( Gui::BitmapFactory().pixmap("Sketcher_Element_Line_StartingPoint") );
    QIcon Sketcher_Element_Point_StartingPoint( Gui::BitmapFactory().pixmap("Sketcher_Element_Point_StartingPoint") );
    QIcon none( Gui::BitmapFactory().pixmap("Sketcher_ConstrainLock") );
    
    for (int i=0;i<ui->listWidgetElements->count(); i++) {
      Base::Type type = static_cast<ElementItem *>(ui->listWidgetElements->item(i))->GeometryType;

      ui->listWidgetElements->item(i)->setIcon(
	(type == Part::GeomPoint::getClassTypeId() 	&& element==1) ? Sketcher_Element_Point_StartingPoint :
	(type == Part::GeomLineSegment::getClassTypeId()  && element==0) ? Sketcher_Element_Line_Edge :
	(type == Part::GeomLineSegment::getClassTypeId()  && element==1) ? Sketcher_Element_Line_StartingPoint :
	(type == Part::GeomLineSegment::getClassTypeId()  && element==2) ? Sketcher_Element_Line_EndPoint :
	(type == Part::GeomArcOfCircle::getClassTypeId() 	&& element==0) ? Sketcher_Element_Arc_Edge :
	(type == Part::GeomArcOfCircle::getClassTypeId() 	&& element==1) ? Sketcher_Element_Arc_StartingPoint :
	(type == Part::GeomArcOfCircle::getClassTypeId() 	&& element==2) ? Sketcher_Element_Arc_EndPoint :
	(type == Part::GeomArcOfCircle::getClassTypeId() 	&& element==3) ? Sketcher_Element_Arc_MidPoint :
	(type == Part::GeomCircle::getClassTypeId()	&& element==0) ? Sketcher_Element_Circle_Edge :
	(type == Part::GeomCircle::getClassTypeId()	&& element==3) ? Sketcher_Element_Circle_MidPoint :
	none);
    }
}

void TaskSketcherElements::changeEvent(QEvent *e)
{
    TaskBox::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(proxy);
    }
}



#include "moc_TaskSketcherElements.cpp"
