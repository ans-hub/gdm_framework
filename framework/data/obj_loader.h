// *************************************************************
// File:    obj_loader.h
// Author:  Novoselov Anton @ 2018 (and author of repo below)
// URL:     https://github.com/ans-hub/gdm_framework
// Fork of: https://github.com/ans-hub/OBJ-Loader.git
// *************************************************************

#if defined(__clang__)
#pragma clang optimize "O3"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC optimize ("O3")
#elif defined(_MSC_VER)
#pragma optimize ("s", on)
#pragma optimize ("g", on)
#pragma optimize ("y", on)
#endif

#ifndef AH_GDM_OBJ_LOADER_H
#define AH_GDM_OBJ_LOADER_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <stdexcept>
#include <cstdlib>

#include "system/string_utils.h"
#include "helpers.h"

// #define OBJL_CONSOLE_OUTPUT

namespace gdm::obj {

struct Except : std::runtime_error
{
  Except(const std::string& msg) : std::runtime_error(msg.c_str()) { }
  Except(const char* msg) : std::runtime_error(msg) { }

}; // struct obj::Except

	// Structure: Vector2
	//
	// Description: A 2D Vector that Holds Positional Data
  struct Vector2
	{
		// Default Constructor
	  Vector2()
		{
		  X = 0.0f;
		  Y = 0.0f;
		}
		// Variable Set Constructor
	  Vector2(float X_, float Y_)
		{
		  X = X_;
		  Y = Y_;
		}
		// Bool Equals Operator Overload
	  bool operator==(const Vector2& other) const
		{
		  return (this->X == other.X && this->Y == other.Y);
		}
		// Bool Not Equals Operator Overload
	  bool operator!=(const Vector2& other) const
		{
		  return !(this->X == other.X && this->Y == other.Y);
		}
		// Addition Operator Overload
	  Vector2 operator+(const Vector2& right) const
		{
		  return Vector2(this->X + right.X, this->Y + right.Y);
		}
		// Subtraction Operator Overload
	  Vector2 operator-(const Vector2& right) const
		{
		  return Vector2(this->X - right.X, this->Y - right.Y);
		}
		// Float Multiplication Operator Overload
	  inline Vector2 operator*(const float& other) const
		{
		  return Vector2(this->X *other, this->Y * other);
		}

		// Positional Variables
	  float X;
	  float Y;
	};

	// Structure: Vector3
	//
	// Description: A 3D Vector that Holds Positional Data
  struct Vector3
	{
		// Default Constructor
	  Vector3()
		{
		  X = 0.0f;
		  Y = 0.0f;
		  Z = 0.0f;
		}
		// Variable Set Constructor
	  Vector3(float X_, float Y_, float Z_)
		{
		  X = X_;
		  Y = Y_;
		  Z = Z_;
		}
		// Bool Equals Operator Overload
	  bool operator==(const Vector3& other) const
		{
		  return (this->X == other.X && this->Y == other.Y && this->Z == other.Z);
		}
		// Bool Not Equals Operator Overload
	  bool operator!=(const Vector3& other) const
		{
		  return !(this->X == other.X && this->Y == other.Y && this->Z == other.Z);
		}
		// Addition Operator Overload
	  Vector3 operator+(const Vector3& right) const
		{
		  return Vector3(this->X + right.X, this->Y + right.Y, this->Z + right.Z);
		}
		// Subtraction Operator Overload
	  Vector3 operator-(const Vector3& right) const
		{
		  return Vector3(this->X - right.X, this->Y - right.Y, this->Z - right.Z);
		}
		// Float Multiplication Operator Overload
	  Vector3 operator*(const float& other) const
		{
		  return Vector3(this->X * other, this->Y * other, this->Z * other);
		}
		// Float Division Operator Overload
	  Vector3 operator/(const float& other) const
		{
		  return Vector3(this->X / other, this->Y / other, this->Z / other);
		}

		// Positional Variables
	  float X;
	  float Y;
	  float Z;
	};

	// Structure: Vertex
	//
	// Description: Model Vertex object that holds
	//  a Position, Normal, and Texture Coordinate
  struct Vertex
	{
		// Position Vector
	  Vector3 Position;

		// Normal Vector
	  Vector3 Normal;

		// Texture Coordinate Vector
	  Vector2 UV;
	};

  struct Material
	{
	  Material()
		{
		  name;
		  Ns = 0.0f;
		  Ni = 0.0f;
		  d = 0.0f;
		  illum = 0;
		}

		// Material Name
	  std::string name;
		// Emissive Color
	  Vector3 Ke;
		// Ambient Color
	  Vector3 Ka;
		// Diffuse Color
	  Vector3 Kd;
		// Specular Color
	  Vector3 Ks;
		// Specular Exponent
	  float Ns;
		// Optical Density
	  float Ni;
		// Dissolve
	  float d;
		// Illumination
	  int illum;
		// Ambient Texture Map
	  std::string map_Ka;
		// Diffuse Texture Map
	  std::string map_Kd;
		// Specular Texture Map
	  std::string map_Ks;
		// Specular Hightlight Map
	  std::string map_Ns;
		// Alpha Texture Map
	  std::string map_d;
		// Bump Map
	  std::string map_bump;
	};

	// Structure: Mesh
	//
	// Description: A Simple Mesh Object that holds
	//  a name, a vertex list, and an index list
  struct Mesh
	{
		// Default Constructor
	  Mesh()
		{

		}
		// Variable Set Constructor
	  Mesh(std::vector<Vertex>& _Vertices, std::vector<unsigned int>& _Indices)
		{
		  Vertices = _Vertices;
		  Indices = _Indices;
		}
		// Mesh Name
	  std::string MeshName;
		// Vertex List
	  std::vector<Vertex> Vertices;
		// Index List
	  std::vector<unsigned int> Indices;

		// Material
	  Material MeshMaterial;
	};

	// Namespace: Math
	//
	// Description: The namespace that holds all of the math
	//  functions need for OBJL
  namespace math
	{
		// Vector3 Cross Product
	  inline Vector3 CrossV3(const Vector3 a, const Vector3 b)
		{
		  return Vector3(a.Y * b.Z - a.Z * b.Y,
			  a.Z * b.X - a.X * b.Z,
			  a.X * b.Y - a.Y * b.X);
		}

		// Vector3 Magnitude Calculation
	  inline float MagnitudeV3(const Vector3 in)
		{
		  return (sqrtf(powf(in.X, 2) + powf(in.Y, 2) + powf(in.Z, 2)));
		}

		// Vector3 DotProduct
	  inline float DotV3(const Vector3 a, const Vector3 b)
		{
		  return (a.X * b.X) + (a.Y * b.Y) + (a.Z * b.Z);
		}

		// Angle between 2 Vector3 Objects
	  inline float AngleBetweenV3(const Vector3 a, const Vector3 b)
		{
		  float angle = DotV3(a, b);
		  angle /= (MagnitudeV3(a) * MagnitudeV3(b));
		  return angle = acosf(angle);
		}

		// Projection Calculation of a onto b
	  inline Vector3 ProjV3(const Vector3 a, const Vector3 b)
		{
		  Vector3 bn = b / MagnitudeV3(b);
		  return bn * DotV3(a, bn);
		}
	}

	// Namespace: Algorithm
	//
	// Description: The namespace that holds all of the
	// Algorithms needed for OBJ
  namespace algorithm
	{
		// Vector3 Multiplication Opertor Overload
	  inline Vector3 operator*(const float& left, const Vector3& right)
		{
		  return Vector3(right.X * left, right.Y * left, right.Z * left);
		}

		// A test to see if P1 is on the same side as P2 of a line segment ab
	  inline bool SameSide(Vector3 p1, Vector3 p2, Vector3 a, Vector3 b)
		{
		  Vector3 cp1 = math::CrossV3(b - a, p1 - a);
		  Vector3 cp2 = math::CrossV3(b - a, p2 - a);

		  if (math::DotV3(cp1, cp2) >= 0)
			  return true;
		  else
			  return false;
		}

		// Generate a cross produect normal for a triangle
	  inline Vector3 GenTriNormal(Vector3 t1, Vector3 t2, Vector3 t3)
		{
		  Vector3 u = t2 - t1;
		  Vector3 v = t3 - t1;

		  Vector3 normal = math::CrossV3(u,v);

		  return normal;
		}

		// Check to see if a Vector3 Point is within a 3 Vector3 Triangle
	  inline bool inTriangle(Vector3 point, Vector3 tri1, Vector3 tri2, Vector3 tri3)
		{
			// Test to see if it is within an infinite prism that the triangle outlines.
		  bool within_tri_prisim = SameSide(point, tri1, tri2, tri3) && SameSide(point, tri2, tri1, tri3)
				&& SameSide(point, tri3, tri1, tri2);

			// If it isn't it will never be on the triangle
		  if (!within_tri_prisim)
			  return false;

			// Calulate Triangle's Normal
		  Vector3 n = GenTriNormal(tri1, tri2, tri3);

			// Project the point onto this normal
		  Vector3 proj = math::ProjV3(point, n);

			// If the distance from the triangle to the point is 0
			//  it lies on the triangle
		  if (math::MagnitudeV3(proj) == 0)
			  return true;
		  else
			  return false;
		}

		// Get tail of string after first token and possibly following spaces
	  inline std::string tail(const std::string &in)
		{
		  size_t token_start = in.find_first_not_of(" \t");
		  size_t space_start = in.find_first_of(" \t", token_start);
		  size_t tail_start = in.find_first_not_of(" \t", space_start);
		  size_t tail_end = in.find_last_not_of(" \t");
		  if (tail_start != std::string::npos && tail_end != std::string::npos)
			{
			  return in.substr(tail_start, tail_end - tail_start + 1);
			}
		  else if (tail_start != std::string::npos)
			{
			  return in.substr(tail_start);
			}
		  return "";
		}

		// Get first token of string
	  inline std::string firstToken(const std::string &in)
		{
		  if (!in.empty())
			{
			  size_t token_start = in.find_first_not_of(" \t");
			  size_t token_end = in.find_first_of(" \t", token_start);
			  if (token_start != std::string::npos && token_end != std::string::npos)
				{
				  return in.substr(token_start, token_end - token_start);
				}
			  else if (token_start != std::string::npos)
				{
				  return in.substr(token_start);
				}
			}
		  return "";
		}

		// Get element at given index position
	  template <class T>
	  inline const T & getElement(const std::vector<T> &elements, const char* index)
		{
		  int idx = atoi(index);
		  if (idx < 0)
			  idx = int(elements.size()) + idx;
		  else
			  idx--;
		  return elements[idx];
		}
	}

  class Loader
	{
  public:
	  Loader() = default;
	  Loader(const std::string& ModelPath, const std::string& TexturePath)
		{
		  if (!LoadFile(ModelPath, TexturePath))
			  throw Except("Coudln't load obj file");
		}
	  bool LoadFile(const std::string& ModelPath, const std::string& TexturePath)
		{
			// If the file is not an .obj file return false
		  if (ModelPath.substr(ModelPath.size() - 4, 4) != ".obj")
			  return false;


		  std::ifstream file(ModelPath);

		  if (!file.is_open())
			  return false;

		  LoadedMeshes.clear();
		  LoadedVertices.clear();
		  LoadedIndices.clear();

		  std::vector<Vector3> Positions;
		  std::vector<Vector2> TCoords;
		  std::vector<Vector3> Normals;

		  std::vector<Vertex> Vertices;
		  std::vector<unsigned int> Indices;

		  std::vector<std::string> MeshMatNames;

		  bool listening = false;
		  std::string meshname;

		  Mesh tempMesh;

			#ifdef OBJL_CONSOLE_OUTPUT
		  const unsigned int outputEveryNth = 1000;
		  unsigned int outputIndicator = outputEveryNth;
			#endif

		  std::string curline;
		  while (std::getline(file, curline))
			{
				#ifdef OBJL_CONSOLE_OUTPUT
			  if ((outputIndicator = ((outputIndicator + 1) % outputEveryNth)) == 1)
				{
				  if (!meshname.empty())
					{
					  std::cout
							<< "\r- " << meshname
							<< "\t| vertices > " << Positions.size()
							<< "\t| texcoords > " << TCoords.size()
							<< "\t| normals > " << Normals.size()
							<< "\t| triangles > " << (Vertices.size() / 3)
							<< (!MeshMatNames.empty() ? "\t| material: " + MeshMatNames.back() : "");
					}
				}
				#endif

				// Generate a Mesh Object or Prepare for an object to be created
			  std::string firstTok = algorithm::firstToken(curline);
				if (firstTok == "o" || firstTok == "g" || curline[0] == 'g')
				{
				  if (!listening)
					{
					  listening = true;

					  if (firstTok == "o" || firstTok == "g")
						{
						  meshname = algorithm::tail(curline);
						}
					  else
						{
						  meshname = "unnamed";
						}
					}
				  else
					{
						// Generate the mesh to put into the array

					  if (!Indices.empty() && !Vertices.empty())
						{
							// Create Mesh
						  tempMesh = Mesh(Vertices, Indices);
						  tempMesh.MeshName = meshname;

							// Insert Mesh
						  LoadedMeshes.push_back(tempMesh);

							// Cleanup
						  Vertices.clear();
						  Indices.clear();
						  meshname.clear();

						  meshname = algorithm::tail(curline);
						}
					  else
						{
						  if (firstTok == "o" || firstTok == "g")
							{
							  meshname = algorithm::tail(curline);
							}
						  else
							{
							  meshname = "unnamed";
							}
						}
					}
					#ifdef OBJL_CONSOLE_OUTPUT
				  std::cout << std::endl;
				  outputIndicator = 0;
					#endif
				}
				// Generate a Vertex Position
			  if (firstTok == "v")
				{
				  Vector3 vpos;
					std::string stail = algorithm::tail(curline);
				  auto spos = gdm::str::Split(stail.c_str(), 0, (int)stail.size(), ' ');
					char* end;
				  vpos.X = std::strtof(spos[0].begin, &end);
				  vpos.Y = std::strtof(spos[1].begin, &end);
				  vpos.Z = std::strtof(spos[2].begin, &end);

				  Positions.push_back(vpos);
				}
				// Generate a Vertex Texture Coordinate
			  if (firstTok[0] == 'v' && firstTok[1] == 't')
				{
				  Vector2 vtex;
					std::string stail = algorithm::tail(curline);
				  auto stex = gdm::str::Split(stail.c_str(), 0, (int)stail.size(), ' ');
					char* end;

				  vtex.X = std::strtof(stex[0].begin, &end);
				  vtex.Y = std::strtof(stex[1].begin, &end);

				  TCoords.push_back(vtex);
				}
				// Generate a Vertex Normal;
			  if (firstTok[0] == 'v' && firstTok[1] == 'n')
				{
				  Vector3 vnor;
					std::string stail = algorithm::tail(curline);
				  auto snor = gdm::str::Split(stail.c_str(), 0, (int)stail.size(), ' ');
					char* end;

				  vnor.X = std::strtof(snor[0].begin, &end);
				  vnor.Y = std::strtof(snor[1].begin, &end);
				  vnor.Z = std::strtof(snor[2].begin, &end);

				  Normals.push_back(vnor);
				}
				// Generate a Face (vertices & indices)
			  if (firstTok == "f")
				{
					// Generate the vertices
				  std::vector<Vertex> vVerts;
				  GenVerticesFromRawOBJ(vVerts, Positions, TCoords, Normals, curline);

					// Add Vertices
				  for (int i = 0; i < int(vVerts.size()); i++)
					{
					  Vertices.push_back(vVerts[i]);

					  LoadedVertices.push_back(vVerts[i]);
					}

				  std::vector<unsigned int> iIndices;

				  VertexTriangluation(iIndices, vVerts);

					// Add Indices
				  for (int i = 0; i < int(iIndices.size()); i++)
					{
					  unsigned int indnum = (unsigned int)((Vertices.size()) - vVerts.size()) + iIndices[i];
					  Indices.push_back(indnum);

					  indnum = (unsigned int)((LoadedVertices.size()) - vVerts.size()) + iIndices[i];
					  LoadedIndices.push_back(indnum);

					}
				}
				// Get Mesh Material Name
			  if (firstTok == "usemtl")
				{
				  MeshMatNames.push_back(algorithm::tail(curline));

					// Create new Mesh, if Material changes within a group
				  if (!Indices.empty() && !Vertices.empty())
					{
						// Create Mesh
					  tempMesh = Mesh(Vertices, Indices);
					  tempMesh.MeshName = meshname;
					  int i = 2;
					  while(1) {
						  tempMesh.MeshName = meshname + "_" + std::to_string(i);

						  for (auto &m : LoadedMeshes)
							  if (m.MeshName == tempMesh.MeshName)
								  continue;
						  break;
						}

						// Insert Mesh
					  LoadedMeshes.push_back(tempMesh);

						// Cleanup
					  Vertices.clear();
					  Indices.clear();
					}

					#ifdef OBJL_CONSOLE_OUTPUT
				  outputIndicator = 0;
					#endif
				}

			  if (firstTok == "mtllib")
				{
				  std::string pathtomat = TexturePath;
				  if (pathtomat.back() != '/')
					  pathtomat.push_back('/');
				  pathtomat += algorithm::tail(curline);

					#ifdef OBJL_CONSOLE_OUTPUT
				  std::cout << std::endl << "- find materials in: " << pathtomat << std::endl;
					#endif
				  LoadMaterials(pathtomat);
				}
			}

			#ifdef OBJL_CONSOLE_OUTPUT
		  std::cout << std::endl;
			#endif

			// Deal with last mesh

		  if (!Indices.empty() && !Vertices.empty())
			{
				// Create Mesh
			  tempMesh = Mesh(Vertices, Indices);
			  tempMesh.MeshName = meshname;

				// Insert Mesh
			  LoadedMeshes.push_back(tempMesh);
			}

		  file.close();

			// Set Materials for each Mesh
		  for (int i = 0; i < MeshMatNames.size(); i++)
			{
				if (i >= LoadedMeshes.size())
					continue;
			  std::string matname = MeshMatNames[i];

				// Find corresponding material name in loaded materials
				// when found copy material variables into mesh material
			  for (int j = 0; j < LoadedMaterials.size(); j++)
				{
				  if (LoadedMaterials[j].name == matname)
					{
					  LoadedMeshes[i].MeshMaterial = LoadedMaterials[j];
					  break;
					}
				}
			}

		  if (LoadedMeshes.empty() && LoadedVertices.empty() && LoadedIndices.empty())
			{
			  return false;
			}
		  else
			{
			  return true;
			}
		}

		// Loaded Mesh Objects
	  std::vector<Mesh> LoadedMeshes;
		// Loaded Vertex Objects
	  std::vector<Vertex> LoadedVertices;
		// Loaded Index Positions
	  std::vector<unsigned int> LoadedIndices;
		// Loaded Material Objects
	  std::vector<Material> LoadedMaterials;

  private:
		// Generate vertices from a list of positions, 
		//  tcoords, normals and a face line
	  void GenVerticesFromRawOBJ(std::vector<Vertex>& oVerts,
		  const std::vector<Vector3>& iPositions,
		  const std::vector<Vector2>& iTCoords,
		  const std::vector<Vector3>& iNormals,
		  const std::string& icurline)
		{
		  Vertex vVert;
			std::string stail = algorithm::tail(icurline);
		  auto sface = gdm::str::Split(stail.c_str(), ' ');
		  bool noNormal = false;

			// For every given vertex do this
		  for (int i = 0; i < int(sface.size()); i++)
			{
				// See What type the vertex is.
			  int vtype;
				int end = (int)(sface[i].end - sface[i].begin);
			  auto svert = gdm::str::Split(sface[i].begin, 0, end, '/', true);

				// Check for just position - v1
			  if (svert.size() == 1)
				{
					// Only position
				  vtype = 1;
				}

				// Check for position & texture - v1/vt1
			  if (svert.size() == 2)
				{
					// Position & Texture
				  vtype = 2;
				}

				// Check for Position, Texture and Normal - v1/vt1/vn1
				// or if Position and Normal - v1//vn1
			  if (svert.size() == 3)
				{
				  if (svert[1].begin != svert[1].end)
					{
						// Position, Texture, and Normal
					  vtype = 4;
					}
				  else
					{
						// Position & Normal
					  vtype = 3;
					}
				}

				// Calculate and store the vertex
			  switch (vtype)
				{
			  case 1: // P
				{
				  vVert.Position = algorithm::getElement(iPositions, svert[0].begin);
				  vVert.UV = Vector2(0, 0);
				  noNormal = true;
				  oVerts.push_back(vVert);
				  break;
				}
			  case 2: // P/T
				{
				  vVert.Position = algorithm::getElement(iPositions, svert[0].begin);
				  vVert.UV = algorithm::getElement(iTCoords, svert[1].begin);
				  noNormal = true;
				  oVerts.push_back(vVert);
				  break;
				}
			  case 3: // P//N
				{
				  vVert.Position = algorithm::getElement(iPositions, svert[0].begin);
				  vVert.UV = Vector2(0, 0);
				  vVert.Normal = algorithm::getElement(iNormals, svert[2].begin);
				  oVerts.push_back(vVert);
				  break;
				}
			  case 4: // P/T/N
				{
				  vVert.Position = algorithm::getElement(iPositions, svert[0].begin);
				  vVert.UV = algorithm::getElement(iTCoords, svert[1].begin);
				  vVert.Normal = algorithm::getElement(iNormals, svert[2].begin);
				  oVerts.push_back(vVert);
				  break;
				}
			  default:
				{
				  break;
				}
				}
			}

			// take care of missing normals
			// these may not be truly acurate but it is the 
			// best they get for not compiling a mesh with normals	
		  if (noNormal)
			{
			  Vector3 A = oVerts[0].Position - oVerts[1].Position;
			  Vector3 B = oVerts[2].Position - oVerts[1].Position;

			  Vector3 normal = math::CrossV3(A, B);

			  for (int i = 0; i < int(oVerts.size()); i++)
				{
				  oVerts[i].Normal = normal;
				}
			}
		}

		// Triangulate a list of vertices into a face by printing
		//  inducies corresponding with triangles within it
	  void VertexTriangluation(std::vector<unsigned int>& oIndices,
		  const std::vector<Vertex>& iVerts)
		{
			// If there are 2 or less verts,
			// no triangle can be created,
			// so exit
		  if (iVerts.size() < 3)
			{
			  return;
			}
			// If it is a triangle no need to calculate it
		  if (iVerts.size() == 3)
			{
			  oIndices.push_back(0);
			  oIndices.push_back(1);
			  oIndices.push_back(2);
			  return;
			}

			// Create a list of vertices
		  std::vector<Vertex> tVerts = iVerts;

		  while (true)
			{
				// For every vertex
			  for (int i = 0; i < int(tVerts.size()); i++)
				{
					// pPrev = the previous vertex in the list
				  Vertex pPrev;
				  if (i == 0)
					{
					  pPrev = tVerts[tVerts.size() - 1];
					}
				  else
					{
					  pPrev = tVerts[i - 1];
					}

					// pCur = the current vertex;
				  Vertex pCur = tVerts[i];

					// pNext = the next vertex in the list
				  Vertex pNext;
				  if (i == tVerts.size() - 1)
					{
					  pNext = tVerts[0];
					}
				  else
					{
					  pNext = tVerts[i + 1];
					}

					// Check to see if there are only 3 verts left
					// if so this is the last triangle
				  if (tVerts.size() == 3)
					{
						// Create a triangle from pCur, pPrev, pNext
					  for (int j = 0; j < int(tVerts.size()); j++)
						{
						  if (iVerts[j].Position == pCur.Position)
							  oIndices.push_back(j);
						  if (iVerts[j].Position == pPrev.Position)
							  oIndices.push_back(j);
						  if (iVerts[j].Position == pNext.Position)
							  oIndices.push_back(j);
						}

					  tVerts.clear();
					  break;
					}
				  if (tVerts.size() == 4)
					{
						// Create a triangle from pCur, pPrev, pNext
					  for (int j = 0; j < int(iVerts.size()); j++)
						{
						  if (iVerts[j].Position == pCur.Position)
							  oIndices.push_back(j);
						  if (iVerts[j].Position == pPrev.Position)
							  oIndices.push_back(j);
						  if (iVerts[j].Position == pNext.Position)
							  oIndices.push_back(j);
						}

					  Vector3 tempVec;
					  for (int j = 0; j < int(tVerts.size()); j++)
						{
						  if (tVerts[j].Position != pCur.Position
								&& tVerts[j].Position != pPrev.Position
								&& tVerts[j].Position != pNext.Position)
							{
							  tempVec = tVerts[j].Position;
							  break;
							}
						}

						// Create a triangle from pCur, pPrev, pNext
					  for (int j = 0; j < int(iVerts.size()); j++)
						{
						  if (iVerts[j].Position == pPrev.Position)
							  oIndices.push_back(j);
						  if (iVerts[j].Position == pNext.Position)
							  oIndices.push_back(j);
						  if (iVerts[j].Position == tempVec)
							  oIndices.push_back(j);
						}

					  tVerts.clear();
					  break;
					}

					// If Vertex is not an interior vertex
				  float angle = math::AngleBetweenV3(pPrev.Position - pCur.Position, pNext.Position - pCur.Position) * (180 / 3.14159265359f);
				  if (angle <= 0 && angle >= 180)
					  continue;

					// If any vertices are within this triangle
				  bool inTri = false;
				  for (int j = 0; j < int(iVerts.size()); j++)
					{
					  if (algorithm::inTriangle(iVerts[j].Position, pPrev.Position, pCur.Position, pNext.Position)
							&& iVerts[j].Position != pPrev.Position
							&& iVerts[j].Position != pCur.Position
							&& iVerts[j].Position != pNext.Position)
						{
						  inTri = true;
						  break;
						}
					}
				  if (inTri)
					  continue;

					// Create a triangle from pCur, pPrev, pNext
				  for (int j = 0; j < int(iVerts.size()); j++)
					{
					  if (iVerts[j].Position == pCur.Position)
						  oIndices.push_back(j);
					  if (iVerts[j].Position == pPrev.Position)
						  oIndices.push_back(j);
					  if (iVerts[j].Position == pNext.Position)
						  oIndices.push_back(j);
					}

					// Delete pCur from the list
				  for (int j = 0; j < int(tVerts.size()); j++)
					{
					  if (tVerts[j].Position == pCur.Position)
						{
						  tVerts.erase(tVerts.begin() + j);
						  break;
						}
					}

					// reset i to the start
					// -1 since loop will add 1 to it
				  i = -1;
				}

				// if no triangles were created
			  if (oIndices.size() == 0)
				  break;

				// if no more vertices
			  if (tVerts.size() == 0)
				  break;
			}
		}

		// Load Materials from .mtl file
	  bool LoadMaterials(const std::string& path)
		{
			// If the file is not a material file return false
		  if (path.substr(path.size() - 4, path.size()) != ".mtl")
			  return false;

		  std::ifstream file(path);

			// If the file is not found return false
		  if (!file.is_open())
			  return false;

		  Material tempMaterial;

		  bool listening = false;

			// Go through each line looking for material variables
		  std::string curline;
		  while (std::getline(file, curline))
			{
				// new material and material name
			  std::string firstTok = algorithm::firstToken(curline);
				if (firstTok == "newmtl")
				{
				  if (!listening)
					{
					  listening = true;

					  if (curline.size() > 7)
						{
						  tempMaterial.name = algorithm::tail(curline);
						}
					  else
						{
						  tempMaterial.name = "none";
						}
					}
				  else
					{
						// Generate the material

						// Push Back loaded Material
					  LoadedMaterials.push_back(tempMaterial);

						// Clear Loaded Material
					  tempMaterial = Material();

					  if (curline.size() > 7)
						{
						  tempMaterial.name = algorithm::tail(curline);
						}
					  else
						{
						  tempMaterial.name = "none";
						}
					}
				}
				// Emissive Color
			  if (firstTok[0] == 'K' && firstTok[1] == 'w')
				{
					std::string stail = algorithm::tail(curline);
				  auto temp = gdm::str::Split(stail.c_str(), ' ');

				  if (temp.size() != 3)
					  continue;

					char* end;
				  tempMaterial.Ke.X = std::strtof(temp[0].begin, &end);
				  tempMaterial.Ke.Y = std::strtof(temp[1].begin, &end);
				  tempMaterial.Ke.Z = std::strtof(temp[2].begin, &end);
				}

				// Ambient Color
			  if (firstTok[0] == 'K' && firstTok[1] == 'a')
				{
					std::string stail = algorithm::tail(curline);
				  auto temp = gdm::str::Split(stail.c_str(), ' ');

				  if (temp.size() != 3)
					  continue;

					char* end;

				  tempMaterial.Ka.X = std::strtof(temp[0].begin, &end);
				  tempMaterial.Ka.Y = std::strtof(temp[1].begin, &end);
				  tempMaterial.Ka.Z = std::strtof(temp[2].begin, &end);
				}
				// Diffuse Color
			  if (firstTok[0] == 'K' && firstTok[1] == 'd')
				{
					std::string stail = algorithm::tail(curline);
				  auto temp = gdm::str::Split(stail.c_str(), ' ');

				  if (temp.size() != 3)
					  continue;

					char* end;
				  tempMaterial.Kd.X = std::strtof(temp[0].begin, &end);
				  tempMaterial.Kd.Y = std::strtof(temp[1].begin, &end);
				  tempMaterial.Kd.Z = std::strtof(temp[2].begin, &end);
				}
				// Specular Color
			  if (firstTok[0] == 'K' && firstTok[1] == 's')
				{
					std::string stail = algorithm::tail(curline);
				  auto temp = gdm::str::Split(stail.c_str(), 0, (int)stail.size(), ' ');
					char* end;

				  if (temp.size() != 3)
					  continue;

				  tempMaterial.Ks.X = std::strtof(temp[0].begin, &end);
				  tempMaterial.Ks.Y = std::strtof(temp[1].begin, &end);
				  tempMaterial.Ks.Z = std::strtof(temp[2].begin, &end);
				}
				// Specular Exponent
			  if (firstTok[0] == 'N' && firstTok[1] == 's')
				{
				  tempMaterial.Ns = std::stof(algorithm::tail(curline));
				}
				// Optical Density
			  if (firstTok[0] == 'N' && firstTok[1] == 'i')
				{
				  tempMaterial.Ni = std::stof(algorithm::tail(curline));
				}
				// Dissolve
			  if (firstTok == "d")
				{
				  tempMaterial.d = std::stof(algorithm::tail(curline));
				}
				// Illumination
			  if (firstTok == "Illum")
				{
				  tempMaterial.illum = std::stoi(algorithm::tail(curline));
				}
				// Ambient Texture Map
			  if (firstTok == "map_Ka")
				{
				  tempMaterial.map_Ka = algorithm::tail(curline);
				}
				// Diffuse Texture Map
			  if (firstTok == "map_Kd")
				{
				  tempMaterial.map_Kd = algorithm::tail(curline);
				}
				// Specular Texture Map
			  if (firstTok == "map_Ks")
				{
				  tempMaterial.map_Ks = algorithm::tail(curline);
				}
				// Specular Hightlight Map
			  if (firstTok == "map_Ns")
				{
				  tempMaterial.map_Ns = algorithm::tail(curline);
				}
				// Alpha Texture Map
			  if (firstTok == "map_d")
				{
				  tempMaterial.map_d = algorithm::tail(curline);
				}
				// Bump Map
			  if (firstTok == "map_Bump" || firstTok == "map_bump" || firstTok == "bump")
				{
				  tempMaterial.map_bump = algorithm::tail(curline);
				}
			}

			// Deal with last material

			// Push Back loaded Material
		  LoadedMaterials.push_back(tempMaterial);

			// Test to see if anything was loaded
			// If not return false
		  if (LoadedMaterials.empty())
			  return false;
			// If so return true
		  else
			  return true;
		}
	}; // class Loader

namespace helpers
{
  template<class T>
    std::vector<T> LoadXYZ(obj::Loader& obj, std::size_t mesh_num);
  template<class T>
    std::vector<T> LoadIndicies(obj::Loader& obj, std::size_t mesh_num);
  template<class T>
    std::vector<T> LoadUV(obj::Loader& obj, std::size_t mesh_num);
  template<class T>
    std::vector<T> LoadNormals(obj::Loader& obj, std::size_t mesh_num);

}; // namespace helpers

template<class T>
std::vector<T> helpers::LoadXYZ(obj::Loader& obj, std::size_t mesh_num)
{
  assert(mesh_num < obj.LoadedMeshes.size());
  assert(!obj.LoadedMeshes[mesh_num].Vertices.empty());

  std::vector<T> result {};
  for (const auto& vx : obj.LoadedMeshes[mesh_num].Vertices)
	  result.emplace_back(T{vx.Position.X, vx.Position.Y, vx.Position.Z});
  return result;
}

template<class T>
std::vector<T> helpers::LoadIndicies(obj::Loader& obj, std::size_t mesh_num)
{
  assert(mesh_num < obj.LoadedMeshes.size());
  assert(!obj.LoadedMeshes[mesh_num].Indices.empty());
  assert(obj.LoadedMeshes[mesh_num].Indices.size() % 3 == 0);

  std::vector<T> result {};
  const auto& indices = obj.LoadedMeshes[mesh_num].Indices;
  for (std::size_t i = 0;  i < indices.size(); i += 3)
	  result.emplace_back(T{indices[i], indices[i+1], indices[i+2]});
  return result;
}

template<class T>
std::vector<T> helpers::LoadUV(obj::Loader& obj, std::size_t mesh_num)
{
  assert(mesh_num < obj.LoadedMeshes.size());
  assert(!obj.LoadedMeshes[mesh_num].Vertices.empty());

  std::vector<T> result {};
  for (const auto& vx : obj.LoadedMeshes[mesh_num].Vertices)
	  result.emplace_back(T{vx.UV.X, vx.UV.Y});
  return result;
}

template<class T>
std::vector<T> helpers::LoadNormals(obj::Loader& obj, std::size_t mesh_num)
{
  assert(mesh_num < obj.LoadedMeshes.size());
  assert(!obj.LoadedMeshes[mesh_num].Vertices.empty());

  std::vector<T> result {};
  for (const auto& vx : obj.LoadedMeshes[mesh_num].Vertices)
	  result.emplace_back(T{vx.Normal.X, vx.Normal.Y, vx.Normal.Z});
  return result;
}

} // namespace gdm::obj

#endif // AH_GDM_OBJ_LOADER_H
