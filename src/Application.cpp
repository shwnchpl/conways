
#include "Application.h"
#include "Config.h"

#include "SDL/SDL.h"

#include <algorithm>
#include <vector>

Application::Application(int width, int height, int cellWidth)
  : m_screenWidth(width),
    m_screenHeight(height),
    m_cellWidth(cellWidth),
    m_stateSpaceWidth(width / cellWidth),
    m_stateSpaceHeight(height / cellWidth)
{
  m_state = new bool[m_stateSpaceWidth * m_stateSpaceHeight]();
}

Application::~Application(void)
{
  delete m_state;
}

bool Application::Init(void)
{
  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_putenv((char*)"SDL_VIDEO_WINDOW_POS=center");

  SDL_WM_SetCaption(WINDOW_TITLE, NULL);

  SDL_AddTimer(TIME_PER_GENERATION_MS,
    [](Uint32 interval, void* param) -> Uint32 {
      SDL_Event event;
      event.type = SDL_USEREVENT;
      SDL_PushEvent(&event);
      return interval;
    }, NULL);

  m_screen = SDL_SetVideoMode(m_screenWidth,
                              m_screenHeight,
                              SCREEN_BPP,
                              SDL_SWSURFACE);
  if (m_screen == nullptr)
    return false;

  m_RGBBlack = SDL_MapRGB(m_screen->format, 0x00, 0x00, 0x00);
  m_RGBWhite = SDL_MapRGB(m_screen->format, 0xff, 0xff, 0xff);

  // Set background color to white.
  SDL_FillRect(m_screen, &m_screen->clip_rect, m_RGBWhite);
  
  // Draw Grid.
  DrawGrid();

  UpdateView();

  return true;
}

void Application::Destroy(void)
{
  SDL_Quit();
}

void Application::DrawGrid(void)
{
  for (Sint16 i = m_cellWidth - 1; i < m_screenWidth; i += m_cellWidth) {
    SDL_Rect line = {0, i, m_screenWidth, 1};

    SDL_FillRect(m_screen, &line, m_RGBBlack);

    std::swap(line.x, line.y);
    std::swap(line.w, line.h);

    SDL_FillRect(m_screen, &line, m_RGBBlack);
  }
}

void Application::UpdateView(void)
{
  SDL_Flip(m_screen);
}

void Application::ToggleCell(int y, int x)
{
  int cellOffset = 0;
  if (GetValidStateSpaceOffset(y, x, cellOffset)) {
    SDL_Rect rect;

    rect.x = x * m_cellWidth;
    rect.y = y * m_cellWidth;
    rect.w = rect.h = m_cellWidth - 1;

    SDL_FillRect(m_screen, &rect, (m_state[cellOffset] = !m_state[cellOffset]) ? m_RGBBlack : m_RGBWhite);
  }
}

int Application::NeighborCount(int y, int x)
{
  int count = 0;
  int offsetOfCell = 0;

  if (GetValidStateSpaceOffset(y, x, offsetOfCell)) {
    if (x > 0) {
      count += m_state[offsetOfCell - 1];
      if (y > 0)
        count += m_state[offsetOfCell - m_stateSpaceWidth - 1];
      if (y  < (m_stateSpaceWidth - 1)) 
        count += m_state[offsetOfCell + m_stateSpaceWidth - 1];
    }

    if (x < (m_stateSpaceWidth - 1)) {
      count += m_state[offsetOfCell + 1];
      if (y > 0)
        count += m_state[offsetOfCell - m_stateSpaceWidth + 1];
      if (y  < (m_stateSpaceWidth - 1)) 
        count += m_state[offsetOfCell + m_stateSpaceWidth + 1];
    }

    if (y > 0)
      count += m_state[offsetOfCell - m_stateSpaceWidth];
    if (y  < (m_stateSpaceWidth - 1)) 
      count += m_state[offsetOfCell + m_stateSpaceWidth];
  }
      
  return count;
}

bool Application::GetValidStateSpaceOffset(int y, int x, int& offset)
{
  int cellOffset = y * m_stateSpaceWidth + x;
  if (cellOffset < (m_stateSpaceWidth * m_stateSpaceHeight)) {
    offset = cellOffset;
    return true;
  }

  return false;
}

void Application::Iterate(void)
{
  std::vector<std::pair<int, int>> cellsToToggle;

  for (int y = 0; y < m_stateSpaceWidth; ++y) {
    for (int x = 0; x < m_stateSpaceWidth; ++x) {
      int offsetOfCell = y * m_stateSpaceWidth + x;
      int neighborCount = NeighborCount(y, x);

      // If alive
      if (m_state[offsetOfCell]) {
        if (neighborCount < 2 || neighborCount > 3)
          cellsToToggle.push_back(std::make_pair(y, x));
      }
      else if (neighborCount == 3) {
        cellsToToggle.push_back(std::make_pair(y, x));
      }
    }
  }

  for (auto& pair : cellsToToggle) {
    ToggleCell(pair.first, pair.second);
  }
}

