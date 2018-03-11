
#include <memory>
#include <algorithm>

#include "SDL/SDL.h"

#define WINDOW_TITLE            "Conway's Game of Life"
#define SCREEN_WIDTH            800
#define SCREEN_HEIGHT           800
#define CELL_WIDTH              20
#define SCREEN_BPP              32
#define TIME_PER_GENERATION_MS  250

// TODO: [ACTIVE] should display in title when game is active.
//       Escape should quit.
//       Command line arguments for speed and size would be nice.

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

int main(void)
{
  SDL_Event event;
  bool done = false, active = false;
  auto app = std::make_unique<Application>(SCREEN_WIDTH, SCREEN_HEIGHT, CELL_WIDTH);

  app->Init();

  while (!done) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
          if (event.button.button == SDL_BUTTON_LEFT) {
            // XXX: This is a little ugly, but it gets the job done.
            app->ToggleCell(event.button.y / CELL_WIDTH, event.button.x / CELL_WIDTH); 
            app->UpdateView();
          }
          break;
        case SDL_KEYDOWN:
          if (event.key.keysym.sym == SDLK_SPACE) {
            active = !active;
          }
          break;
        case SDL_USEREVENT:
          if (active) {
            app->Iterate();
            app->UpdateView();
          }
          break;
        case SDL_QUIT:
          done = true;
          break;
        default:
          break;
      }
    }
  }

  app->Destroy();
  app.reset(nullptr);

  return 0;
}
