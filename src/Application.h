#if !defined(__CONWAYS_APPLICATION_H_)
#define __CONWAYS_APPLICATION_H_

#include "SDL/SDL.h"

class Application {
public:
  Application(int width, int height, int cellWidth);
  ~Application(void);

  bool Init(void);
  void Destroy(void);
  void UpdateView(void);
  void ToggleCell(int y, int x);
  void Iterate(void);

private:
  void DrawGrid(void);
  int NeighborCount(int y, int x);
  bool GetValidStateSpaceOffset(int y, int x, int& offset);

  Uint32  m_RGBBlack;
  Uint32  m_RGBWhite;

  Uint16  m_screenWidth;
  Uint16  m_screenHeight;
  int     m_cellWidth;
  int     m_stateSpaceWidth;
  int     m_stateSpaceHeight;

  bool*         m_state;

  SDL_Surface*  m_screen; 
};

#endif
