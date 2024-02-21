#include <iostream>
#include <string>
#include "allocation.h"
#include "fem_define.h"
#include "vtkCellType.h"
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkHexahedron.h>
#include <vtkTetra.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkXMLUnstructuredGridWriter.h>

// #################################################################
/**
 * @brief export the results.
 * @param node      : 節点情報
 * @param element   : 接続情報
 * @param numOfNode : 総節点数
 * @param numOfElm  : 総要素数
 * @param pointData : 節点で定義される物理量（ここでは3Dベクトル）
 * @param cellData  : 要素で定義される物理量（ここではスカラー）
 * @param filename  : 出力ファイルの名前
 * @param mode      : ascii, binary, appended
 */
void VTK_ExportAsVTU(ARRAY2D<double> &node, const elementType &element, int numOfNode, int numOfElm,
                     ARRAY2D<double> &pointData, ARRAY1D<double> &cellData,
                     const string filename, const string mode) 
{
  vtkSmartPointer<vtkUnstructuredGrid> grid = vtkSmartPointer<vtkUnstructuredGrid>::New();

  ///// set points /////
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  for(int i = 0; i < numOfNode; i++) points->InsertNextPoint(node(i,0),node(i,1),node(i,2));
  grid->SetPoints(points);
  //////////////////////

  ///// set cells /////
  auto setConnectivity = [&element,&grid](const int ic) {
    if(element[ic].meshType == VTK_HEXAHEDRON) {
      vtkSmartPointer<vtkHexahedron> hexa = vtkSmartPointer<vtkHexahedron>::New();
      for(int i = 0; i < element[ic].node.size(); i++) hexa->GetPointIds()->SetId(i,element[ic].node[i]);
      grid->InsertNextCell(hexa->GetCellType(),hexa->GetPointIds());
    }
    else if(element[ic].meshType == VTK_TETRA) {
      vtkSmartPointer<vtkTetra> tetra = vtkSmartPointer<vtkTetra>::New();
      for(int i = 0; i < element[ic].node.size(); i++) tetra->GetPointIds()->SetId(i,element[ic].node[i]);
      grid->InsertNextCell(tetra->GetCellType(),tetra->GetPointIds());
    }
    else {
      std::cout << "Undefined mesh type!\n";
      exit(EXIT_FAILURE);
    }
  };
  for(int ic = 0; ic < numOfElm; ic++) setConnectivity(ic);
  /////////////////////

  ////// set point data /////
  auto setPointData3D = [&grid,numOfNode](ARRAY2D<double> &data, string name) {
    vtkSmartPointer<vtkFloatArray> array = vtkSmartPointer<vtkFloatArray>::New();
    array->SetName(name.c_str());
    array->SetNumberOfComponents(3);
    for(int i = 0; i < numOfNode; i++) array->InsertNextTuple3(data(i,0),data(i,1),data(i,2));
    grid->GetPointData()->AddArray(array);
  };
  setPointData3D(pointData, "変数名");
  ///////////////////////////

  ///// set cell data /////
  auto setCellData1D = [&grid,numOfElm](ARRAY1D<double> &data, string name) {
    vtkSmartPointer<vtkFloatArray> array = vtkSmartPointer<vtkFloatArray>::New();
    array->SetName(name.c_str());
    array->SetNumberOfComponents(1);
    for(int i = 0; i < numOfElm; i++) array->InsertNextValue(data(i));
    grid->GetCellData()->AddArray(array);
  };
  setCellData1D(cellData, "変数名");
  /////////////////////////

  ///// export /////
  vtkSmartPointer<vtkXMLUnstructuredGridWriter> writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
  writer->SetFileName(filename.c_str());
  if(mode == "ascii")
    writer->SetDataModeToAscii();
  else if(mode == "binary")
    writer->SetDataModeToBinary();
  else if(mode == "appended")
    writer->SetDataModeToAppended();
  else {
    std::cout << "invarid mode!\n";
    exit(EXIT_FAILURE);
  }
  writer->SetInputData(grid);
  writer->Write();
  //////////////////
}