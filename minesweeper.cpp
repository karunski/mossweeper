#include <c64.h>
#include <stdio.h>
#include <string.h>
#include <key_scan.h>
#include <sid.h>
#include <music_player.h>
#include <pla.h>
#include "find.h"

extern "C" {
extern c64::Sprite sprite_data_ram[32];
extern c64::Sprite sprite_data_end;
extern const std::byte minesweeper_gfx[2048];
extern const c64::Sprite minesweeper_cursor[7];
extern const c64::Sprite minesweeper_bg_sprites[1];
extern const c64::ColorCode minesweeper_color[256];
//extern std::byte char_data_ram[2048];
}

namespace{

using ScreenMemoryAddresses =
    c64::DisplayAddr<c64::CIA2::vic_bank::NO_3, c64::VicII::VIDEO_OFFSET_0000, c64::VicII::TEXT_0800>;
c64::ScreenRAM &screen_ram = ScreenMemoryAddresses::screen;
c64::CharRAM & char_data_ram = ScreenMemoryAddresses::chars;

constexpr auto BLANK = c64::ScreenCode{1};
constexpr std::uint8_t ROWS_MAX = 16;
constexpr std::uint8_t COLUMNS_MAX = 30;

std::uint8_t game_rows = 0;
std::uint8_t game_columns = 0;
std::uint8_t mines = 0;

void clear_screen() {
  auto *screen_mem = screen_ram.data();

  static auto CLEAR_COLOR = minesweeper_color[static_cast<size_t>(BLANK)];

  for (unsigned i = 0; i < 1000; i += 1) {
    screen_mem[i] = BLANK;
  }

  for (std::uint8_t i = 0; i < 8; i += 1) {
    c64::vic_ii.set_sprite_pos(i, 0, 0);
  }
  }

  struct key_scan_res {
    bool space : 1;
    bool w : 1;
    bool a : 1;
    bool s : 1;
    bool d : 1;
  };

  key_scan_res check_keys() {
    c64::JoyScanner joyscan;

    if (const auto joystick_state = joyscan.scan_joysticks();
        joystick_state.joystick_a != c64::JoyScanner::JoyScan::CLEAR ||
        joystick_state.joystick_b != c64::JoyScanner::JoyScan::CLEAR) {
      return key_scan_res{
          c64::JoyScanner::JoyScan::BUTTON & joystick_state.joystick_a,
          c64::JoyScanner::JoyScan::UP & joystick_state.joystick_a,
          c64::JoyScanner::JoyScan::LEFT & joystick_state.joystick_a,
          c64::JoyScanner::JoyScan::DOWN & joystick_state.joystick_a,
          c64::JoyScanner::JoyScan::RIGHT & joystick_state.joystick_a};
    }

    c64::KeyScanner scanner;
    const auto scanRow0 = scanner.Row<0>();
    const auto scanRow1 = scanner.Row<1>();
    const auto scanRow2 = scanner.Row<2>();
    const auto scanRow6 = scanner.Row<6>();
    const auto scanRow7 = scanner.Row<7>();
    const auto down = scanRow0.KEY_CURSOR_DOWN();
    const auto right = scanRow0.KEY_CURSOR_RIGHT();
    const auto shift = scanRow1.KEY_LEFT_SHIFT() || scanRow6.KEY_RIGHT_SHIFT();
    return key_scan_res{scanRow7.KEY_SPACE() || scanRow0.KEY_RETURN(),
                        scanRow1.KEY_W() || (down && shift),
                        scanRow1.KEY_A() || (right && shift),
                        scanRow1.KEY_S() || (down && !shift),
                        scanRow2.KEY_D() || (right && !shift)};
  }

  template<class TileType, std::uint8_t Width, std::uint8_t Height>
  struct MetaTile
  {
    static constexpr std::uint8_t width = Width;
    static constexpr std::uint8_t height = Height;

    using RowType = TileType[Width];

    const RowType tiles[Height];
  };

  struct SpriteBase {
  public:
    static constexpr std::uint8_t sprite_x_offset = 24;
    static constexpr std::uint8_t sprite_y_offset = 50;

    struct Position {
      std::uint16_t X;
      std::uint8_t Y;
    };

    void enable(bool en) { c64::vic_ii.sprite_enable.set(active_number, en); }

    void multicolor_enable(bool en) {
      c64::vic_ii.sprite_multicolor_enable.set(active_number, en);
    }

    void data_priority(bool en) {
      c64::vic_ii.sprite_data_priority.set(active_number, en);
    }

    void position(std::uint16_t x, std::uint8_t y) {
      c64::vic_ii.set_sprite_pos(active_number, x, y);
    }

    void expand(bool x, bool y) {
      c64::vic_ii.sprite_x_expansion.set(active_number, x);
      c64::vic_ii.sprite_y_expansion.set(active_number, y);
    }

  protected:
    std::uint8_t active_number = 0;
  };

  template <std::uint8_t Slot, std::uint8_t Frames>
  struct Sprite : public SpriteBase
  {
    static constexpr std::uint8_t slot = Slot;
    static constexpr std::uint8_t frames = Frames;

    void activate(std::uint8_t sprite_number, c64::ColorCode sprite_color) {
      active_number = sprite_number;
      screen_ram.sprite_ptr(active_number) = sprite_data_ram[slot];
      c64::vic_ii.sprite_color[active_number] =
          sprite_color;
    }

    void select_frame(std::uint8_t frame) {
      screen_ram.sprite_ptr(active_number) =
          sprite_data_ram[slot + frame];
    }

  };

  using Cursor = Sprite<0, 7>;
  Cursor cursor;
  std::uint8_t cursor_anim_frame = 0;

  using SpriteBackground = Sprite<7, 1>;
  SpriteBackground sprite_background;

  using C64Emoji = MetaTile<c64::ScreenCode, 2, 2>;
  using C64Digit = MetaTile<c64::ScreenCode, 1, 2>;

  struct TilePoint {

    TilePoint left(std::uint8_t distance = 1) const {
      return TilePoint{static_cast<std::uint8_t>(X - distance), Y};
    }

    TilePoint right(std::uint8_t distance = 1) const {
      return TilePoint{static_cast<std::uint8_t>(X + distance), Y};
    }

    TilePoint up(std::uint8_t distance = 1) const {
      return TilePoint{X, static_cast<std::uint8_t>(Y - distance)};
    }

    TilePoint down(std::uint8_t distance = 1) const {
      return TilePoint{X, static_cast<std::uint8_t>(Y + distance)};
    }

    bool operator==(const TilePoint & rhs) const {
      return X == rhs.X && Y == rhs.Y;
    }

    std::uint8_t X;
    std::uint8_t Y;
  };

  constexpr TilePoint board_pos{1, 1};

  struct C64GameBoardTraits
  {
    using TileType = c64::ScreenCode;
    static constexpr auto TopLeft = TileType{8};
    static constexpr auto TopRight = TileType{5};
    static constexpr auto BottomLeft = TileType{39};
    static constexpr auto BottomRight = TileType{36};
    static constexpr auto TopBorder = TileType{4};
    static constexpr auto RightBorder = TileType{37};
    static constexpr auto BottomBorder = TileType{6};
    static constexpr auto LeftBorder = TileType{7};
    static constexpr auto ScoreRows = std::uint8_t{2};

    static constexpr auto HiddenSquare = TileType{0};

    // assume ExposedSquare doubles as '0' marker; and '1', '2', ... '8'
    // are in the tiles following it.
    static constexpr auto ExposedSquare = TileType{16};

    static constexpr auto NumberMarker(std::uint8_t idx) {
      return static_cast<TileType>(static_cast<std::uint8_t>(ExposedSquare) + idx);
    }

    static constexpr auto Mine = TileType{25};
    static constexpr auto Flag = TileType{26};
    static constexpr auto Wrong = TileType{27};

    static void place(TileType Tile, std::uint8_t x, std::uint8_t y) {
      screen_ram.at(x, y) = Tile;
      c64::color_ram.at(x, y) = minesweeper_color[static_cast<uint8_t>(Tile)];
    }

    static void place(TileType Tile, TilePoint tilePos)
    {
      place(Tile, tilePos.X, tilePos.Y);
    }

    static constexpr C64Emoji Happy = {
        {{TileType{2}, TileType{3}}, {TileType{34}, TileType{35}}}};

    static constexpr C64Emoji Caution = {
        {{TileType{49}, TileType{50}}, {TileType{51}, TileType{52}}}};

    static constexpr C64Emoji Dead = {
        {{TileType{53}, TileType{54}}, {TileType{55}, TileType{56}}}};

    static constexpr C64Emoji Win = {
        {{TileType{57}, TileType{58}}, {TileType{34}, TileType{35}}}};

    static constexpr C64Digit ScoreDigits[10] = {
        {{TileType{15}, TileType{46}}}, {{TileType{9}, TileType{41}}},
        {{TileType{10}, TileType{42}}}, {{TileType{10}, TileType{43}}},
        {{TileType{11}, TileType{45}}}, {{TileType{12}, TileType{43}}},
        {{TileType{12}, TileType{44}}}, {{TileType{13}, TileType{41}}},
        {{TileType{14}, TileType{44}}}, {{TileType{14}, TileType{43}}}};

    static constexpr auto LetterA = TileType{64};
    static constexpr auto SelectArrow = TileType{32};
  };

  static_assert(C64GameBoardTraits::Happy.tiles[0][1] == c64::ScreenCode{3});
  static_assert(C64GameBoardTraits::Happy.tiles[1][0] == c64::ScreenCode{34});

  template<class GameBoardTraits>
  class GameBoardDraw
  {
    using TileType = typename GameBoardTraits::TileType;

    template<TileType LeftCorner, TileType Middle, TileType RightCorner>
    static void DrawBorderRow(std::uint8_t currentRow)
    {
      GameBoardTraits::place(LeftCorner, board_pos.X, currentRow);
      for (std::uint8_t i = 0; i < game_columns; i += 1)
      {
        GameBoardTraits::place(Middle, board_pos.X + 1 + i, currentRow);
      }
      GameBoardTraits::place(RightCorner, board_pos.X + 1 + game_columns,
                             currentRow);
    }

    static void DrawSideBorders(std::uint8_t currentRow)
    {
      GameBoardTraits::place(GameBoardTraits::LeftBorder, board_pos.X, currentRow);
      GameBoardTraits::place(GameBoardTraits::RightBorder, board_pos.X + 1 + game_columns, currentRow);
    }

    static void DrawHidden(std::uint8_t currentRow)
    {
      for (std::uint8_t i = 1; i <= game_columns; i += 1) {
        GameBoardTraits::place(GameBoardTraits::HiddenSquare, board_pos.X + i, currentRow);
      }
    }

    template<class MetaTileType>
    static void DrawMetaTile(const MetaTileType & metaTile, std::uint8_t x, std::uint8_t y)
    {
      for (std::uint8_t i = 0; i < MetaTileType::height; i += 1)
      {
        for (std::uint8_t j = 0; j < MetaTileType::width; j += 1)
        {
          GameBoardTraits::place(metaTile.tiles[i][j], x + j, y + i);
        }
      }
    }

  public:
    static void Draw000(std::uint8_t x_off, std::uint16_t val) {
      DrawMetaTile(GameBoardTraits::ScoreDigits[val / 100], x_off,
                   board_pos.Y + 1);
      val %= 100;
      DrawMetaTile(GameBoardTraits::ScoreDigits[val / 10], x_off + 1,
                   board_pos.Y + 1);
      val %= 10;
      DrawMetaTile(GameBoardTraits::ScoreDigits[val], x_off + 2,
                   board_pos.Y + 1);
    }

    static TilePoint SelectionToTilePosition(TilePoint game_selection) {
      return {static_cast<std::uint8_t>(board_pos.X + 1 + game_selection.X),
              static_cast<std::uint8_t>(board_pos.Y + 1 + GameBoardTraits::ScoreRows + game_selection.Y)};
    }

    using Traits = GameBoardTraits;

    static void DrawBoard() {
      static std::uint8_t currentRow;

      currentRow = board_pos.Y;

      DrawBorderRow<GameBoardTraits::TopLeft, GameBoardTraits::TopBorder,
                    GameBoardTraits::TopRight>(currentRow);

      currentRow += 1;

      for (std::uint8_t i = 0; i < GameBoardTraits::ScoreRows; i += 1) {
        DrawSideBorders(currentRow);
        currentRow += 1;
      }

      for (std::uint8_t i = 0; i < game_rows; i += 1) {
        DrawSideBorders(currentRow);
        DrawHidden(currentRow);
        currentRow += 1;
      }

      DrawBorderRow<GameBoardTraits::BottomLeft, GameBoardTraits::BottomBorder,
                    GameBoardTraits::BottomRight>(currentRow);

      DrawResetButtonHappy();

      Draw000(board_pos.X + 1, 0);
      Draw000(board_pos.X + 1 + game_columns - 3, 0);
    }

    static void DrawScore(std::uint8_t score) {
      Draw000(board_pos.X + 1, score);
    }
    static void DrawTime(std::uint16_t seconds) {
      Draw000(board_pos.X + 1 + game_columns - 3, seconds);
    }

    static void DrawResetButtonHappy() {
      DrawMetaTile(GameBoardTraits::Happy, reset_button_x(), reset_button_y());
    }

    static void DrawResetButtonCaution() {
      DrawMetaTile(GameBoardTraits::Caution, reset_button_x(), reset_button_y());
    }

    static void DrawResetButtonDead() {
      DrawMetaTile(GameBoardTraits::Dead, reset_button_x(), reset_button_y());
    }

    static void DrawResetButtonWin() {
      DrawMetaTile(GameBoardTraits::Win, reset_button_x(), reset_button_y());
    }

    static void ExposeTile(TilePoint tile)
    {
      GameBoardTraits::place(
          GameBoardTraits::ExposedSquare, SelectionToTilePosition(tile));
    }

    static void Mine(TilePoint tile)
    {
      GameBoardTraits::place(GameBoardTraits::Mine, SelectionToTilePosition(tile));
    }

    static void Wrong(TilePoint tile) {
      GameBoardTraits::place(GameBoardTraits::Wrong,
                             SelectionToTilePosition(tile));
    }

    static void Flag(TilePoint tile)
    {
      GameBoardTraits::place(GameBoardTraits::Flag, SelectionToTilePosition(tile));
    }

    static void Hide(TilePoint tile)
    {
      GameBoardTraits::place(GameBoardTraits::HiddenSquare, SelectionToTilePosition(tile));
    }

    static std::uint8_t ShowCount(std::uint8_t count, TilePoint where)
    {
      GameBoardTraits::place(GameBoardTraits::NumberMarker(count),
                             SelectionToTilePosition(where));
      return count;
    }

    static constexpr std::uint8_t LeftBoardLimit() { return 1; }
    static constexpr std::uint8_t RightBoardLimit(std::uint8_t columns) {
      return columns;
    }
    static constexpr std::uint8_t TopBoardLimit() {
      return GameBoardTraits::ScoreRows + 1;
    }
    static constexpr std::uint8_t BottomBoardLimit(std::uint8_t rows) {
      return TopBoardLimit() + rows - 1;
    }

    static std::uint16_t tile_to_sprite_x(std::uint8_t tile_x) {
      return Cursor::sprite_x_offset + (unsigned{tile_x} << 3u);
    }

    static std::uint8_t tile_to_sprite_y(std::uint8_t tile_y) {
      return Cursor::sprite_y_offset + (tile_y << 3u);
    }

    static std::uint16_t selection_to_sprite_x(std::uint8_t tile_x) {
      return tile_to_sprite_x(board_pos.X + LeftBoardLimit() + tile_x);
    }

    static std::uint8_t selection_to_sprite_y(std::uint8_t tile_y) {
      return tile_to_sprite_y(board_pos.Y + TopBoardLimit() + tile_y);
    }

    static std::uint8_t reset_button_x() {
      return board_pos.X + 1 + (game_columns / 2) - (GameBoardTraits::Happy.width / 2);
    }

    static std::uint8_t reset_button_y() { return board_pos.Y + 1; }

    static void DrawString(const char * str, std::uint8_t x,
                             std::uint8_t y) {
      for (std::uint8_t i = 0; *(str+i); i += 1) {
        GameBoardTraits::place(
            static_cast<TileType>(static_cast<std::uint8_t>(GameBoardTraits::LetterA) +
                     (*(str + i) - 'A')),
            x + i, y);
      }
    }
  };

  struct RowBits {
    std::byte m_bits[(COLUMNS_MAX >> 3) + static_cast<bool>(COLUMNS_MAX & 0x7)];

    bool test(std::uint8_t position_x) const {
      const auto byte = m_bits[position_x >> 3];
      return (byte & (std::byte{1} << (position_x & 0x7))) != std::byte{0};
    }

    bool set(std::uint8_t position_x, bool value = true) {
      const std::byte set_pattern = std::byte{1} << (position_x & 0x7);
      if (value) {
        m_bits[position_x >> 3] |= set_pattern;
      } else {
        m_bits[position_x >> 3] &= ~(set_pattern);
      }

      return value;
    }
  };

  struct ExposeResultContinuation;
  struct GameState {
    using BitVector = RowBits[ROWS_MAX];
    BitVector mine_bits;
    BitVector exposed_bits;
    BitVector flag_bits;
    std::uint16_t mines_left : 7;
    std::uint16_t hidden_clear : 9;
    bool time_running : 1;
    std::uint16_t timer: 15;
    const ExposeResultContinuation *expose_continuation;

    static std::uint8_t count_bits(const BitVector & state_bits, TilePoint selection)
    {
      return state_bits[selection.Y].test(selection.X);
    }

    std::uint8_t count_mine(TilePoint selection) const {
      return count_bits(mine_bits, selection);
    }

    bool exposed_test_and_set(TilePoint selection) {
      auto & rowbits = exposed_bits[selection.Y];
      if (rowbits.test(selection.X)) {
        return true;
      }
      
      rowbits.set(selection.X);
      
      return false;
    }

    static std::uint8_t count_around(const BitVector & state_bits, TilePoint selection) {
      const bool has_column_left = (selection.X > 0);
      const bool has_column_right = (selection.X < (game_columns - 1));
      return (selection.Y > 0
                  ? (count_bits(state_bits, selection.up()) +
                     (has_column_left
                          ? count_bits(state_bits, selection.up().left())
                          : 0) +
                     (has_column_right
                          ? count_bits(state_bits, selection.up().right())
                          : 0))
                  : 0) +
             (has_column_left ? count_bits(state_bits, selection.left()) : 0) +
             (has_column_right ? count_bits(state_bits, selection.right())
                               : 0) +
             (selection.Y < (game_rows - 1)
                  ? (count_bits(state_bits, selection.down()) +
                     (has_column_left
                          ? count_bits(state_bits, selection.down().left())
                          : 0) +
                     (has_column_right
                          ? count_bits(state_bits, selection.down().right())
                          : 0))
                  : 0);
    }

    std::uint8_t count_mines_around(TilePoint selection) {
      return count_around(mine_bits, selection);
    }

    std::uint8_t count_flags_around(TilePoint selection) {
      return count_around(flag_bits, selection);
    }

    bool set_flag(TilePoint selection) {
      auto & row = flag_bits[selection.Y];
      const bool is_setting_flag = !row.test(selection.X);
      mines_left += is_setting_flag ? -1 : 1;
      return row.set(selection.X, !row.test(selection.X));
    }

    bool is_flagged(TilePoint selection) {
      return count_bits(flag_bits, selection);
    }

    void reset() {
      timer = 0;
      time_running = false;
      mines_left = mines;
      memset(mine_bits, 0, sizeof(mine_bits));
      memset(exposed_bits, 0, sizeof(exposed_bits));
      memset(flag_bits, 0, sizeof(flag_bits));
      expose_continuation = nullptr;
    }

  };

  static GameState game_state{};

  using GameBoardDrawer = GameBoardDraw<C64GameBoardTraits>;

  bool bad_flag_at(TilePoint selection) {
    const auto flagged = game_state.is_flagged(selection);
    const auto is_mine = game_state.count_mine(selection);
    if (flagged && !is_mine) {
      GameBoardDrawer::Wrong(selection);
    }
    else if (is_mine) {
      GameBoardDrawer::Mine(selection);
      return false;
    }
    return true;
  }

  bool bad_around_selection(TilePoint board_selection) {
    bool is_bad = false;

    if (board_selection.Y > 0) {
      bad_flag_at(board_selection.up()) || (is_bad = true);
      board_selection.X > 0 && (bad_flag_at(board_selection.up().left()) || (is_bad = true));
      board_selection.X < (game_columns - 1) && (bad_flag_at(board_selection.up().right()) || (is_bad = true));
    }
    board_selection.X > 0 && (bad_flag_at(board_selection.left()) || (is_bad = true));
    board_selection.X < (game_columns - 1) && (bad_flag_at(board_selection.right()) || (is_bad = true));

    if (board_selection.Y < (game_rows - 1))
    {
      bad_flag_at(board_selection.down()) || (is_bad = true);
      board_selection.X > 0 && (bad_flag_at(board_selection.down().left()) || (is_bad = true));
      board_selection.X < (game_columns - 1) && (bad_flag_at(board_selection.down().right()) || (is_bad = true));
    }

    return !is_bad;
  }

  struct expose_result {
    bool is_ok;
    const ExposeResultContinuation *next_expose;
  };

  struct ExposeResultContinuation {
    virtual expose_result operator()() const = 0;
  };

  struct ExposeResultDelay : public ExposeResultContinuation {
    virtual expose_result operator()() const {
      if (m_skip_count--) {
        return {true, this};
      }

      return {true, m_next};
    }

    static std::uint8_t                     m_skip_count;
    static const ExposeResultContinuation * m_next;
  };

  std::uint8_t ExposeResultDelay::m_skip_count = 0;
  const ExposeResultContinuation * ExposeResultDelay::m_next = nullptr;

  ExposeResultDelay expose_delay;

  struct OptionalTilePoint : public TilePoint {

    OptionalTilePoint() = default;
    OptionalTilePoint(TilePoint p) : TilePoint{p}, is_valid{true} {}

    OptionalTilePoint left() const {
      return is_valid && X > 0 ? OptionalTilePoint{TilePoint::left()} : OptionalTilePoint{};
    }

    OptionalTilePoint right() const {
      return is_valid && X < (game_columns - 1) ? OptionalTilePoint{TilePoint::right()} : OptionalTilePoint{};
    }

    OptionalTilePoint up() const {
      return is_valid && Y > 0 ? OptionalTilePoint{TilePoint::up()} : OptionalTilePoint{};
    }

    OptionalTilePoint down() const {
      return is_valid && Y < (game_rows - 1) ? OptionalTilePoint{TilePoint::down()} : OptionalTilePoint{};
    }

    bool is_valid = false;
  };

  OptionalTilePoint filter_already_exposed(const OptionalTilePoint & src) {
    return src.is_valid && !game_state.exposed_bits[src.Y].test(src.X) ? src : OptionalTilePoint{};
  }

  struct ExposeBuffer {
    TilePoint data[COLUMNS_MAX + ROWS_MAX];
    TilePoint * m_end = data;

    void push_back(OptionalTilePoint optional_tile_point) {
      if (optional_tile_point.is_valid && m_end != data + std::size(data)) {
        *m_end = static_cast<const TilePoint &>(optional_tile_point);
        m_end += 1;
      }
    }

    TilePoint pop_back() {
      return *(--m_end);
    }

    void clear() { m_end = data; }

    bool empty() const { return m_end == data; }

    auto size() const { return m_end - data; }

    const TilePoint *begin() const { return data; }
    const TilePoint *end() const { return m_end; }
  };

  ExposeBuffer expose_buffers[2];
  ExposeBuffer *back_expose_buffer = &expose_buffers[0];
  ExposeBuffer *front_expose_buffer = &expose_buffers[1];
  ExposeBuffer copy_temp;

  void swap_expose_buffers() {
    const auto temp = back_expose_buffer;
    back_expose_buffer = front_expose_buffer;
    front_expose_buffer = temp;
  }

  OptionalTilePoint
  filter_if_in_buffers(const OptionalTilePoint &board_selection_optional) {
    if (!board_selection_optional.is_valid ||
        std::find(front_expose_buffer->begin(), front_expose_buffer->end(),
                  static_cast<const TilePoint &>(board_selection_optional)) !=
            front_expose_buffer->end()||
        std::find(back_expose_buffer->begin(), back_expose_buffer->end(),
                  static_cast<const TilePoint &>(board_selection_optional)) !=
            back_expose_buffer->end()) {
      return OptionalTilePoint{};
    }

    return board_selection_optional;
  }

  void append_back_expose_buffer(const OptionalTilePoint & board_selection_optional) {
    {
      const auto left_tile = board_selection_optional.left();
      back_expose_buffer->push_back(
          filter_if_in_buffers(filter_already_exposed(left_tile.down())));
      back_expose_buffer->push_back(
          filter_if_in_buffers(filter_already_exposed(left_tile)));
      back_expose_buffer->push_back(
          filter_if_in_buffers(filter_already_exposed(left_tile.up())));
    }

    back_expose_buffer->push_back(filter_if_in_buffers(
        filter_already_exposed(board_selection_optional.up())));

    {
      const auto right_tile = board_selection_optional.right();
      back_expose_buffer->push_back(
          filter_if_in_buffers(filter_already_exposed(right_tile.up())));
      back_expose_buffer->push_back(
          filter_if_in_buffers(filter_already_exposed(right_tile)));
      back_expose_buffer->push_back(
          filter_if_in_buffers(filter_already_exposed(right_tile.down())));
    }

    back_expose_buffer->push_back(filter_if_in_buffers(
        filter_already_exposed(board_selection_optional.down())));
  }

  struct ExposeResultNext : public ExposeResultContinuation {
    expose_result operator()() const override {

      back_expose_buffer->clear();

      constexpr std::uint8_t MAX_EXPOSE_PER_ITER = 1;
      std::uint8_t exposed = 0;

      while (!front_expose_buffer->empty()) {

        const auto expose_target = front_expose_buffer->pop_back();

        if (game_state.exposed_test_and_set(expose_target)) {
          continue;
        }

        // bad-flag check.
        if (game_state.is_flagged(expose_target)) {
          if (!game_state.count_mine(expose_target)) {
            return {bad_flag_at(expose_target), nullptr};
          }
          continue;
        }

        const auto mine_count = game_state.count_mines_around(expose_target);
        if (game_state.count_flags_around(expose_target) > mine_count) {
          return {bad_around_selection(expose_target), nullptr};
        }

        GameBoardDrawer::ShowCount(mine_count, expose_target);
        exposed += 1;

        game_state.hidden_clear -= 1;

        if (mine_count == 0)
        {
          append_back_expose_buffer(expose_target);
        }

        // Early exit by limiting the max number of "expose" tiles that change in the current
        // invocation of this call.
        if (exposed == MAX_EXPOSE_PER_ITER)
        {
          // copy the un-traversed region of the front buffer to the back
          // buffer, so it will be examined at some point in the future.
          for (const auto & unexposed_tile : *front_expose_buffer)
          {
            back_expose_buffer->push_back(unexposed_tile);
          }

          break;
        }
      }

      swap_expose_buffers();

      return {true, front_expose_buffer->empty() ? nullptr : this};
    }
  };

  const ExposeResultNext expose_must_continue;

  expose_result
  expose_recurse(TilePoint board_selection, std::uint8_t depth) {

    const auto flagged = game_state.is_flagged(board_selection);
    if (game_state.count_mine(board_selection) && !flagged) {
      GameBoardDrawer::Mine(board_selection);
      return {false, nullptr};
    }

    const auto already_exposed = game_state.exposed_test_and_set(board_selection);

    if (flagged)
    {
      return {true, nullptr};
    }

    static constexpr uint8_t already_exposed_unhide_count[] = { 1, 0 };
    game_state.hidden_clear -= already_exposed_unhide_count[already_exposed];

    const auto mine_count = game_state.count_mines_around(board_selection);
    const auto flag_count = game_state.count_flags_around(board_selection);

    GameBoardDrawer::ShowCount(mine_count, board_selection);

    if (mine_count == 0 || (already_exposed && flag_count == mine_count)) {
      front_expose_buffer->clear();
      back_expose_buffer->clear();
      append_back_expose_buffer(board_selection);
      swap_expose_buffers();
      return {true, &expose_must_continue};
    }

    return {true, nullptr};
  }

  struct ClockUpdater {
    std::uint8_t frames_per_second = 0;
    std::uint8_t current_frames = 0;
    bool operator()() {
      if (++current_frames == frames_per_second) {
        game_state.timer += 1;
        current_frames = 0;
      }

      GameBoardDrawer::DrawTime(game_state.timer);
      return true;
    }
  };

  ClockUpdater clock_updater;

  class FireButtonEventFilter {
  public:
    FireButtonEventFilter() : pressed{false}, down_count{0} {}

    enum Event : std::uint8_t {
      NO_EVENT,
      PRESS,
      LONG_PRESS,
      RELEASE,
    };

    Event operator()(const bool current_state) {
      if (pressed) {
        if (current_state) {
          pressed = true;
          down_count += 1;
          if (down_count >= (clock_updater.frames_per_second >> 1)) {
            down_count = 0;
            return LONG_PRESS;
            }
          return NO_EVENT;
        } else {
          pressed = false;
          down_count = 0;
          return RELEASE;
        }
      } else {
        pressed = current_state;
        if (current_state) {
          return PRESS;
        }

        return NO_EVENT;
      }
    }

  private:
    bool pressed : 1;
    std::uint8_t down_count : 7;
  };

  class CursorAnimateFunc {
    public:
    void operator()() {
      static constexpr std::uint8_t cursor_frameskip = 1;
      static std::uint8_t cursor_current_frameskip = 0;

      if (cursor_current_frameskip++ == cursor_frameskip) {
        cursor_current_frameskip = 0;

        cursor_anim_frame += 1;
        if (cursor_anim_frame == Cursor::frames) {
          cursor_anim_frame = 0;
        }
      }

      cursor.select_frame(cursor_anim_frame);
    }
  };

  class ScoreUpdate {
    public:
    void operator()() {
      GameBoardDrawer::DrawScore(game_state.mines_left);
    }
  };

  ScoreUpdate score_updater;

  void reset() {
    GameBoardDrawer::DrawBoard();

    sprite_background.position(
        GameBoardDrawer::tile_to_sprite_x(GameBoardDrawer::reset_button_x()),
        GameBoardDrawer::tile_to_sprite_y(GameBoardDrawer::reset_button_y()));

    static constexpr std::uint8_t cursor_frameskip = 1;
    static std::uint8_t cursor_current_frameskip = 0;

    c64::sid.voices[2].set_frequency(0xFFFF);
    c64::sid.voices[2].set_control(c64::SIDVoice::noise);

    game_state.reset();

    for (std::uint8_t mines_left = mines; mines_left;) {
      const auto rownum = c64::sid.oscillator_3 % game_rows;
      auto &row = game_state.mine_bits[rownum];
      const auto column = c64::sid.oscillator_3 % game_columns;
      if (!row.test(column)) {
        row.set(column);
        mines_left -= 1;
      }
    }

  }

  CursorAnimateFunc cursor_animator;

  FireButtonEventFilter fire_button_handler;

  class DirectionEventFilter
  {
    public:

    key_scan_res operator()(key_scan_res poll) {
      static constexpr std::uint8_t KEY_REPEAT_DELAY = 9;

      key_scan_res result{};

      static constexpr auto down_count_update = [](std::uint8_t previous_count,
                                                   bool current_state) {
        const std::uint8_t choose_down_count[] = {
            0, static_cast<std::uint8_t>(previous_count + 1)};
        return choose_down_count[current_state && previous_count <= KEY_REPEAT_DELAY];
      };

      m_up_down_count = down_count_update(m_up_down_count, poll.w);
      m_left_down_count = down_count_update(m_left_down_count, poll.a);
      m_down_down_count = down_count_update(m_down_down_count, poll.s);
      m_right_down_count = down_count_update(m_right_down_count, poll.d);
      result.w = (poll.w && !m_last_key_state.w) || m_up_down_count == KEY_REPEAT_DELAY;
      result.a = (poll.a && !m_last_key_state.a) || m_left_down_count == KEY_REPEAT_DELAY;
      result.s = (poll.s && !m_last_key_state.s) || m_down_down_count == KEY_REPEAT_DELAY;
      result.d = (poll.d && !m_last_key_state.d) || m_right_down_count == KEY_REPEAT_DELAY;
      m_last_key_state = poll;
      return result;
    }

    private:
    key_scan_res m_last_key_state;
    std::uint8_t m_up_down_count;
    std::uint8_t m_left_down_count;
    std::uint8_t m_down_down_count;
    std::uint8_t m_right_down_count;
  };

  DirectionEventFilter direction_event_filter{};

  struct AppMode {
    virtual AppMode *on_vsync(FireButtonEventFilter::Event, key_scan_res) = 0;
  };

  struct AppModeGame : public AppMode {

    AppMode * init() {
      current_selected = TilePoint{};
      return this;
    }

    AppMode *on_vsync(FireButtonEventFilter::Event, key_scan_res) override;

    static TilePoint current_selected;
  };

  TilePoint AppModeGame::current_selected{0, 0};

  struct AppModeResetButton : public AppMode {
    AppMode *on_vsync(FireButtonEventFilter::Event, key_scan_res) override;
  };

  struct AppModeSelectDifficulty : public AppMode {

    static void draw() {
      GameBoardDrawer::DrawString("SELECT DIFFICULTY ", 1, 3);
      GameBoardDrawer::DrawString("BEGINNER", 5, 5);
      GameBoardDrawer::DrawString("INTERMEDIATE", 5, 7);
      GameBoardDrawer::DrawString("EXPERT", 5, 9);
    }

    AppMode * on_vsync(FireButtonEventFilter::Event, key_scan_res) override;

    enum Difficulty : std::uint8_t {
      BEGINNER = 0, INTERMEDIATE = 1, EXPERT = 2
    };

    static constexpr TilePoint DifficultyToSelectionArrow[] = {
      TilePoint{3, 5}, TilePoint{3, 7}, TilePoint{3, 9}
    };

    static Difficulty difficulty;
  };

  AppModeSelectDifficulty::Difficulty AppModeSelectDifficulty::difficulty = AppModeSelectDifficulty::BEGINNER;

  struct AppModeDead : public AppMode {
    AppMode *on_vsync(FireButtonEventFilter::Event, key_scan_res) override;
  };

  struct AppModeWin : public AppMode {
    AppMode *on_vsync(FireButtonEventFilter::Event, key_scan_res) override;
  };

  AppModeGame game_field;
  AppModeResetButton reset_selection;
  AppModeDead mode_dead;
  AppModeWin mode_win;

  constexpr AppMode * next_mode_win[2] = { &game_field, &mode_win};

  static constexpr c64::Note shoot_sfx[] = {
      {c64::SIDVoice::noise | c64::SIDVoice::gate, 0x28c8, 3},
      {c64::SIDVoice::noise, 0, 1}};

  static constexpr c64::Note expose_sfx[] = {
      {c64::SIDVoice::triangle | c64::SIDVoice::gate, c64::Note::A_SHARP_4, 10},
      {c64::SIDVoice::triangle | c64::SIDVoice::gate, c64::Note::C_4, 5},
      {c64::SIDVoice::triangle, 0, 1}};

  static constexpr c64::Note flag_sfx[] = {
      {c64::SIDVoice::triangle | c64::SIDVoice::gate, c64::Note::C_3, 13},
      {c64::SIDVoice::ControlFlags{}, 0, 1}};

  AppMode *
  AppModeGame::on_vsync(FireButtonEventFilter::Event fire_button_events,
                        key_scan_res direction_events) {

    // Keeps track of cases where we are clearing a flag and don't want
    // to expose on fire-up.
    static bool suppress_expose = false;

    // "expose" means we are figuring out which tiles need to be automatically
    // opened up because there are no mines around them. Figuring out which
    // ones are like this can take a long time, so we split the operation up 
    // into a sequence of function calls that expose only a few tiles per frame.
    // Each of these function object pointers is stored in game_state.expose_continuation
    // for the current frame.
    if (game_state.expose_continuation)
    {
      const auto [is_ok, next_expose] = (*game_state.expose_continuation)();
      game_state.expose_continuation = next_expose;

      if (!is_ok) {
        game_state.time_running = false;
        return &mode_dead;
      }

      suppress_expose = game_state.expose_continuation != nullptr;
    }

    // on space bar released...
    switch (fire_button_events) {
    case FireButtonEventFilter::RELEASE:
      if (suppress_expose) {
        suppress_expose = false;
      }
      else {
        const auto [is_ok, next_expose] = expose_recurse(current_selected, 0);
        game_state.expose_continuation = next_expose;

        if (!is_ok) {
          game_state.time_running = false;
          return &mode_dead;
        }
      }
      GameBoardDrawer::DrawResetButtonHappy();
      game_state.time_running = true;
      break;
    case FireButtonEventFilter::LONG_PRESS: {
      const bool flag_is_set = game_state.set_flag(current_selected);
      if (flag_is_set) {
        c64::MusicPlayer::play(0, false, flag_sfx);
        GameBoardDrawer::Flag(current_selected);
      } else {
        suppress_expose = true;
        GameBoardDrawer::Hide(current_selected);
      }
    } break;
    case FireButtonEventFilter::PRESS:
      c64::MusicPlayer::play(0, false, expose_sfx);
      GameBoardDrawer::DrawResetButtonCaution();
      break;
    case FireButtonEventFilter::NO_EVENT:
      break;
    }

    if (direction_events.w || direction_events.a || direction_events.s || direction_events.d) {
      c64::MusicPlayer::play(1, false, shoot_sfx);
    }

    if ((direction_events.w && current_selected.Y == 0) ||
        (direction_events.s && current_selected.Y == game_rows - 1)) {
      return &reset_selection;
    }

    current_selected =
        current_selected.up(direction_events.w && current_selected.Y > 0)
            .down(direction_events.s && current_selected.Y < game_rows - 1)
            .right((current_selected.X == 0 && direction_events.a) *
                   game_columns)
            .left(direction_events.a)
            .left(
                (current_selected.X == game_columns - 1 && direction_events.d) *
                game_columns)
            .right(direction_events.d);

    cursor.position(GameBoardDrawer::selection_to_sprite_x(current_selected.X),
                    GameBoardDrawer::selection_to_sprite_y(current_selected.Y));
    cursor.expand(false, false);

    cursor_animator();

    score_updater();

    const auto winning = game_state.hidden_clear == 0;
    game_state.time_running = !winning;
    return next_mode_win[winning];
  }

  AppMode *
  AppModeResetButton::on_vsync(FireButtonEventFilter::Event fire_button_events,
                               key_scan_res direction_events) {
    
    switch (fire_button_events) {
      case FireButtonEventFilter::RELEASE:
        reset();
        break;
      case FireButtonEventFilter::PRESS:
        GameBoardDrawer::DrawResetButtonCaution();
        break;
      case FireButtonEventFilter::LONG_PRESS:
      case FireButtonEventFilter::NO_EVENT:
        break;
    }

    cursor.position(
        GameBoardDrawer::tile_to_sprite_x(GameBoardDrawer::reset_button_x()),
        GameBoardDrawer::tile_to_sprite_y(GameBoardDrawer::reset_button_y()));
    cursor.expand(true, true);

    cursor_animator();

    if (direction_events.s) {
      AppModeGame::current_selected.Y = 0;
      return &game_field;
    }

    if (direction_events.w) {
      AppModeGame::current_selected.Y = game_rows - 1;
      return &game_field;
    }
    
    return this;
  }

  AppMode *AppModeSelectDifficulty::on_vsync(
      FireButtonEventFilter::Event fire_button_events,
      key_scan_res direction_events) {

    struct DifficultySettings
    {
      std::uint8_t m_rows;
      std::uint8_t m_columns;
      std::uint8_t m_mines;
      std::uint16_t m_hidden_clear;

      constexpr DifficultySettings(std::uint8_t rows, std::uint8_t columns,
                                   std::uint8_t mines)
          : m_rows{rows}, m_columns{columns}, m_mines{mines},
            m_hidden_clear{static_cast<uint16_t>(rows) *
                               static_cast<uint16_t>(columns) -
                           static_cast<uint16_t>(mines)} {}
    };

    static constexpr DifficultySettings DIFFICULTY_PRESETS[] = {
        {9, 9, 10}, {16,16,10}, {16,30,99}
    };

    switch (fire_button_events) {
    case FireButtonEventFilter::RELEASE:
      game_rows = DIFFICULTY_PRESETS[difficulty].m_rows;
      game_columns = DIFFICULTY_PRESETS[difficulty].m_columns;
      mines = DIFFICULTY_PRESETS[difficulty].m_mines;
      game_state.hidden_clear = DIFFICULTY_PRESETS[difficulty].m_hidden_clear;
      clear_screen();
      reset();
      return game_field.init(); 
      break;
    case FireButtonEventFilter::LONG_PRESS: 
    case FireButtonEventFilter::PRESS:
    case FireButtonEventFilter::NO_EVENT:
      break;
    }

    switch (difficulty)
    {
      case BEGINNER:
        difficulty = direction_events.s ? INTERMEDIATE : BEGINNER;
        break;
      case INTERMEDIATE:
        difficulty = direction_events.w
                         ? BEGINNER
                         : (direction_events.s ? EXPERT : INTERMEDIATE);
        break;
      case EXPERT:
        difficulty = direction_events.w ? INTERMEDIATE : EXPERT;
        break;
    }
    
    GameBoardDrawer::Traits::place(BLANK, DifficultyToSelectionArrow[BEGINNER]);
    GameBoardDrawer::Traits::place(BLANK,
                                   DifficultyToSelectionArrow[INTERMEDIATE]);
    GameBoardDrawer::Traits::place(BLANK, DifficultyToSelectionArrow[EXPERT]);
    GameBoardDrawer::Traits::place(GameBoardDrawer::Traits::SelectArrow,
                                   DifficultyToSelectionArrow[difficulty]);

    return this;
  }

  AppModeSelectDifficulty difficulty_selection;

  AppMode *current_mode = &difficulty_selection;

  AppMode * AppModeDead::on_vsync(FireButtonEventFilter::Event fire_button_events, key_scan_res) {
    switch (fire_button_events) {
    case FireButtonEventFilter::RELEASE:
      clear_screen();
      difficulty_selection.draw();
      return &difficulty_selection;
      break;
    case FireButtonEventFilter::LONG_PRESS:
    case FireButtonEventFilter::PRESS:
    case FireButtonEventFilter::NO_EVENT:
      break;
    }

    GameBoardDrawer::DrawResetButtonDead();

    cursor.position(
        GameBoardDrawer::tile_to_sprite_x(GameBoardDrawer::reset_button_x()),
        GameBoardDrawer::tile_to_sprite_y(GameBoardDrawer::reset_button_y()));
    cursor.expand(true, true);

    cursor_animator();

    return this;
  }

  AppMode *AppModeWin::on_vsync(FireButtonEventFilter::Event fire_button_events,
                                key_scan_res) {
    switch (fire_button_events) {
    case FireButtonEventFilter::RELEASE:
      clear_screen();
      difficulty_selection.draw();
      return &difficulty_selection;
      break;
    case FireButtonEventFilter::LONG_PRESS:
    case FireButtonEventFilter::PRESS:
    case FireButtonEventFilter::NO_EVENT:
      break;
    }

    GameBoardDrawer::DrawResetButtonWin();

    cursor.position(
        GameBoardDrawer::tile_to_sprite_x(GameBoardDrawer::reset_button_x()),
        GameBoardDrawer::tile_to_sprite_y(GameBoardDrawer::reset_button_y()));
    cursor.expand(true, true);

    cursor_animator();

    return this;
  }

  std::uint16_t count_raster() {
    static std::uint16_t count_raster;
    for (count_raster = 0;;) {
      const auto poll_raster = c64::vic_ii.get_raster();
      if (poll_raster >= count_raster) {
        count_raster = poll_raster;
      } else {
        return count_raster;
      }
    }
  }

  constexpr c64::Note scale[] = {
      {c64::SIDVoice::triangle | c64::SIDVoice::gate, c64::Note::C_4, 25},
      {c64::SIDVoice::triangle, c64::Note::C_4, 5},
      c64::Note{c64::SIDVoice::triangle | c64::SIDVoice::gate, c64::Note::D_4,
                25},
      c64::Note{c64::SIDVoice::triangle, c64::Note::D_4, 5},
      c64::Note{c64::SIDVoice::triangle | c64::SIDVoice::gate, c64::Note::E_4,
                25},
      c64::Note{c64::SIDVoice::triangle, c64::Note::E_4, 5},
      c64::Note{c64::SIDVoice::triangle | c64::SIDVoice::gate, c64::Note::F_4,
                25},
      c64::Note{c64::SIDVoice::triangle, c64::Note::F_4, 5},
      c64::Note{c64::SIDVoice::triangle | c64::SIDVoice::gate, c64::Note::G_4,
                25},
      c64::Note{c64::SIDVoice::triangle, c64::Note::G_4, 5},
      c64::Note{c64::SIDVoice::triangle | c64::SIDVoice::gate, c64::Note::A_4,
                25},
      c64::Note{c64::SIDVoice::triangle, c64::Note::A_4, 5},
      c64::Note{c64::SIDVoice::triangle | c64::SIDVoice::gate, c64::Note::B_4,
                25},
      c64::Note{c64::SIDVoice::triangle, c64::Note::B_4, 5},
      c64::Note{c64::SIDVoice::triangle | c64::SIDVoice::gate, c64::Note::C_5,
                25},
      c64::Note{c64::SIDVoice::triangle, c64::Note::C_5, 5},
  };

} // namespace

int main()
{
  if (&char_data_ram != reinterpret_cast<void *>(0xC800)) {
    puts("WRONG CHAR MEMORY OFFSET");
    return 1;
  }

  clock_updater.frames_per_second = count_raster() > 263 ? 50 : 60;

  const auto current_mmap = c64::pla.get_cpu_lines();

  c64::cia2.set_vic_bank(ScreenMemoryAddresses::vic_base_setting);
  memcpy(char_data_ram.data, minesweeper_gfx, sizeof(minesweeper_gfx));

  {
    const c64::ScopedInterruptDisable disable_interrupts;
    const c64::PLA::BankSwitchScope disable_io {c64::PLA::MODE_28};

    // Put the cursor sprite in VRAM.
    for (std::uint8_t i = 0; i < Cursor::frames; i += 1) {
      sprite_data_ram[Cursor::slot + i] = minesweeper_cursor[i];
    }

    sprite_data_ram[SpriteBackground::slot] = minesweeper_bg_sprites[0];
  }

  c64::vic_ii.setup_memory(ScreenMemoryAddresses::screen_setting, ScreenMemoryAddresses::char_data_setting);
  c64::vic_ii.background_color[0] = c64::ColorCode::WHITE;
  c64::vic_ii.set_multi_color_mode(false);

  // cursor shall be sprite 0.
  cursor.activate(0, minesweeper_cursor[0].mode.sprite_color());
  cursor.enable(true);
  cursor.data_priority(false);
  cursor.multicolor_enable(false);

  sprite_background.activate(7, minesweeper_bg_sprites[0].mode.sprite_color());
  sprite_background.enable(true);
  sprite_background.data_priority(true);
  sprite_background.multicolor_enable(false);

  auto vsync_waiter = c64::get_vsync_wait();

  clear_screen();
  AppModeSelectDifficulty::draw();

  c64::sid.clear();
  c64::sid.set_volume(c64::Nibble{15}, c64::SID::VolumeBits{});
  c64::sid.voices[0].set_attack_decay(c64::Nibble{0}, c64::Nibble{9});
  c64::sid.voices[0].set_sustain_release(c64::Nibble{15}, c64::Nibble{1});
  c64::sid.voices[1].set_attack_decay(c64::Nibble{0}, c64::Nibble{6});
  c64::sid.voices[1].set_sustain_release(c64::Nibble{0}, c64::Nibble{0});

  while (true) {

    vsync_waiter();

    static key_scan_res keys;
    keys = check_keys();

    current_mode = current_mode->on_vsync(fire_button_handler(keys.space),
                                          direction_event_filter(keys));

    game_state.time_running &&clock_updater();

    c64::MusicPlayer::update();
  }

  return 0;
}
