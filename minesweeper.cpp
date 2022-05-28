#include "platform_switch.h"
#include <string.h>
#include <cstdint>
#include <cstddef>
#include <typeinfo>
#include "find.h"
#include "rand.h"
#include "tile_model.h"
#include "input_model.h"

namespace {

constexpr std::uint8_t ROWS_MAX = 16;
constexpr std::uint8_t COLUMNS_MAX = 30;

std::uint8_t game_rows = 0;
std::uint8_t game_columns = 0;
std::uint8_t mines = 0;

target::graphics::sprite cursor;
std::uint8_t cursor_anim_frame = 0;

#ifdef PLATFORM_C64
  target::graphics::sprite sprite_background;
#endif

  struct OptionalTilePoint : public TilePoint {

    OptionalTilePoint() = default;
    OptionalTilePoint(TilePoint p) : TilePoint{p}, is_valid{true} {}

    OptionalTilePoint left() const {
      return is_valid && X > 0 ? OptionalTilePoint{TilePoint::left()}
                               : OptionalTilePoint{};
    }

    OptionalTilePoint right() const {
      return is_valid && X < (game_columns - 1)
                 ? OptionalTilePoint{TilePoint::right()}
                 : OptionalTilePoint{};
    }

    OptionalTilePoint up() const {
      return is_valid && Y > 0 ? OptionalTilePoint{TilePoint::up()}
                               : OptionalTilePoint{};
    }

    OptionalTilePoint down() const {
      return is_valid && Y < (game_rows - 1)
                 ? OptionalTilePoint{TilePoint::down()}
                 : OptionalTilePoint{};
    }

    bool is_valid = false;
  };

  template<class GameBoardTraits>
  class GameBoardDraw
  {
    using TileType = typename GameBoardTraits::tile_type;

    template<TileType LeftCorner, TileType Middle, TileType RightCorner>
    static void DrawBorderRow(std::uint8_t currentRow)
    {
      GameBoardTraits::place_immediate(LeftCorner, board_pos.X, currentRow);
      GameBoardTraits::fill_immediate(Middle, board_pos.X+1, currentRow, game_columns);
      GameBoardTraits::place_immediate(
          RightCorner, board_pos.X + 1 + game_columns, currentRow);
    }

    static void DrawSideBorders(std::uint8_t currentRow)
    {
      GameBoardTraits::place_immediate(GameBoardTraits::LeftBorder, board_pos.X, currentRow);
      GameBoardTraits::place_immediate(GameBoardTraits::RightBorder,
                                       board_pos.X + 1 + game_columns,
                                       currentRow);
    }

    static void DrawHidden(std::uint8_t currentRow)
    {
      GameBoardTraits::fill_immediate(GameBoardTraits::HiddenSquare, board_pos.X + 1, currentRow, game_columns);
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

    static void CenterBoardOnScreen() {
      board_pos.X = (GameBoardTraits::ScreenWidth - (game_columns + 2)) / 2;
      board_pos.Y = (GameBoardTraits::ScreenHeight -
                     (game_rows + GameBoardTraits::ScoreRows + 2)) /
                    2;
    }

    static TilePoint board_pos;

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

    static TilePoint SelectionToTilePosition(const TilePoint & game_selection) {
      return {static_cast<std::uint8_t>(board_pos.X + 1 + game_selection.X),
              static_cast<std::uint8_t>(board_pos.Y + 1 + GameBoardTraits::ScoreRows + game_selection.Y)};
    }

    static void SetGameSize(std::uint8_t rows, std::uint8_t columns) {
      game_rows = rows;
      game_columns = columns;
      CenterBoardOnScreen();
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

    static void ExposeTile(const TilePoint & tile)
    {
      GameBoardTraits::place(
          GameBoardTraits::ExposedSquare, SelectionToTilePosition(tile));
    }

    static void Mine(const TilePoint & tile)
    {
      GameBoardTraits::place(GameBoardTraits::Mine, SelectionToTilePosition(tile));
    }

    static void Wrong(const TilePoint & tile) {
      GameBoardTraits::place(GameBoardTraits::Wrong,
                             SelectionToTilePosition(tile));
    }

    static void Flag(const TilePoint & tile)
    {
      GameBoardTraits::place(GameBoardTraits::Flag, SelectionToTilePosition(tile));
    }

    static void Hide(const TilePoint & tile)
    {
      GameBoardTraits::place(GameBoardTraits::HiddenSquare, SelectionToTilePosition(tile));
    }

    static std::uint8_t ShowCount(std::uint8_t count, const TilePoint & where)
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
      return target::graphics::sprite::sprite_x_offset + (unsigned{tile_x} << 3u);
    }

    static std::uint8_t tile_to_sprite_y(std::uint8_t tile_y) {
      return target::graphics::sprite::sprite_y_offset + (tile_y << 3u);
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

    template <std::uint8_t len>
    static void
    DrawString(const TilePattern<target::graphics::chr_code_type, len> &pattern,
               std::uint8_t x, std::uint8_t y) {
      GameBoardTraits::place_immediate(pattern.m_data, len, x, y);
    }

    template <uint8_t len>
    static constexpr auto GenerateTileString(const char (&str)[len]) {
      return transform_string<target::graphics::chr_code_type, target::graphics::tile_to_char>(
          str);
    }
  };

  template <class GameBoardTraits>
  TilePoint GameBoardDraw<GameBoardTraits>::board_pos{0, 0};

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
    std::uint8_t mines_left;
    std::uint16_t hidden_clear;
    bool time_running;
    std::uint16_t timer;
    const ExposeResultContinuation *expose_continuation;

    static std::uint8_t count_bits(const BitVector & state_bits, const TilePoint & selection)
    {
      return state_bits[selection.Y].test(selection.X);
    }

    static std::uint8_t count_bits(const BitVector &state_bits,
                                   const OptionalTilePoint & selection) {
      if (!selection.is_valid) {
        return 0;
      }

      return state_bits[selection.Y].test(selection.X);
    }

    std::uint8_t count_mine(const TilePoint & selection) const {
      return count_bits(mine_bits, selection);
    }

    bool exposed_test_and_set(const TilePoint & selection) {
      auto & rowbits = exposed_bits[selection.Y];
      if (rowbits.test(selection.X)) {
        return true;
      }
      
      rowbits.set(selection.X);
      
      return false;
    }

    static std::uint8_t count_around(const BitVector & state_bits, const OptionalTilePoint & selection) {
      const auto tile_up = selection.up();
      const auto tile_down = selection.down();
      return count_bits(state_bits, tile_up) +
             count_bits(state_bits, tile_up.left()) +
             count_bits(state_bits, tile_up.right()) +
             count_bits(state_bits, selection.left()) +
             count_bits(state_bits, selection.right()) +
             count_bits(state_bits, tile_down) +
             count_bits(state_bits, tile_down.left()) +
             count_bits(state_bits, tile_down.right());
    }

    std::uint8_t count_mines_around(const TilePoint & selection) {
      return count_around(mine_bits, selection);
    }

    std::uint8_t count_flags_around(const TilePoint & selection) {
      return count_around(flag_bits, selection);
    }

    bool set_flag(const TilePoint & selection) {
      auto & row = flag_bits[selection.Y];
      const bool is_setting_flag = !row.test(selection.X);
      mines_left += is_setting_flag ? -1 : 1;
      return row.set(selection.X, !row.test(selection.X));
    }

    bool is_flagged(const TilePoint & selection) {
      return count_bits(flag_bits, selection);
    }

    bool is_exposed(const TilePoint & selection) {
      return count_bits(exposed_bits, selection);
    }

    void reset() {
      timer = 0;
      time_running = false;
      mines_left = mines;
      memset(mine_bits, 0, sizeof(mine_bits));
      memset(exposed_bits, 0, sizeof(exposed_bits));
      memset(flag_bits, 0, sizeof(flag_bits));
      expose_continuation = nullptr;
      hidden_clear = (game_rows * game_columns) - mines;
    }

  };

  static GameState game_state{};

  using GameBoardDrawer = GameBoardDraw<target::graphics>;

  bool bad_flag_at(const TilePoint & selection) {
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

  bool bad_around_selection(const TilePoint & board_selection) {
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

  OptionalTilePoint filter_already_exposed(const OptionalTilePoint & src) {
    return src.is_valid && !game_state.exposed_bits[src.Y].test(src.X) ? src : OptionalTilePoint{};
  }

  class ExposeBuffer {
    static_assert(COLUMNS_MAX + ROWS_MAX < 256);

    static constexpr std::uint8_t STATIC_SIZE = COLUMNS_MAX + ROWS_MAX;

    TilePoint data[STATIC_SIZE];
    std::uint8_t m_begin = 0;
    std::uint8_t m_size = 0;

    std::uint8_t end_index() const {
      return (m_begin + m_size) % STATIC_SIZE;
    }
    TilePoint & end_ref() {
      return data[end_index()];
    }

  public:
    void push_back(const OptionalTilePoint & optional_tile_point) {
      if (optional_tile_point.is_valid && m_size < STATIC_SIZE) {
        end_ref() = optional_tile_point;
        m_size += 1;
      }
    }

    void push_front(const OptionalTilePoint & optional_tile_point) {
      if (optional_tile_point.is_valid && m_size < STATIC_SIZE) {
        m_begin = m_begin == 0 ? STATIC_SIZE - 1 : m_begin - 1;
        data[m_begin] = optional_tile_point;
        m_size += 1;
      }
    }

    TilePoint pop_back() {
      --m_size;
      return end_ref();
    }

    void clear() { 
      m_begin = 0; 
      m_size = 0;
    }

    auto size() const { return m_size; }
    bool empty() const { return size() == 0; }

    class const_iterator {
    private:
      std::uint8_t logical_index = 0;
      const ExposeBuffer * m_container = nullptr;

    public:
      const_iterator(const ExposeBuffer *container) : m_container{container} {}
      const_iterator(const ExposeBuffer *container, std::uint8_t idx)
          : m_container{container}, logical_index{idx} {}

      const TilePoint & operator*() const {
        return m_container->data[(m_container->m_begin + logical_index) % STATIC_SIZE];
      }

      const_iterator & operator++() {
        ++logical_index;
        return *this;
      }

      bool operator==(const const_iterator & other) const {
        return logical_index == other.logical_index;
      }

      bool operator!=(const const_iterator & other) const {
        return !operator==(other);
      }
    };

    auto begin() const { return const_iterator{this}; }
    auto end() const { return const_iterator{this, m_size}; }

  };

  ExposeBuffer expose_buffer;
  TilePoint prev_exposed;

  OptionalTilePoint filter_around_prev_expose(const OptionalTilePoint &src) {
    const auto x_distance = src.X > prev_exposed.X ? src.X - prev_exposed.X
                                                   : prev_exposed.X - src.X;
    const auto y_distance = src.Y > prev_exposed.Y ? src.Y - prev_exposed.Y
                                                   : prev_exposed.Y - src.Y;
    return src.is_valid && (x_distance > 1 || y_distance > 1)
               ? src
               : OptionalTilePoint{};
  }

  void
  append_back_expose_buffer(const OptionalTilePoint &board_selection_optional) {
    {
      const auto left_tile = board_selection_optional.left();
      expose_buffer.push_front(
          filter_already_exposed(filter_around_prev_expose(left_tile.down())));
      expose_buffer.push_front(
          filter_already_exposed(filter_around_prev_expose(left_tile)));
      expose_buffer.push_front(
          filter_already_exposed(filter_around_prev_expose(left_tile.up())));
    }

    expose_buffer.push_front(filter_already_exposed(
        filter_around_prev_expose(board_selection_optional.up())));

    {
      const auto right_tile = board_selection_optional.right();
      expose_buffer.push_front(
          filter_already_exposed(filter_around_prev_expose(right_tile.up())));
      expose_buffer.push_front(
          filter_already_exposed(filter_around_prev_expose(right_tile)));
      expose_buffer.push_front(
          filter_already_exposed(filter_around_prev_expose(right_tile.down())));
    }

    expose_buffer.push_front(filter_already_exposed(
        filter_around_prev_expose(board_selection_optional.down())));
  }

  void
  init_back_expose_buffer(const OptionalTilePoint &board_selection_optional) {
    
    {
      const auto left_tile = board_selection_optional.left();
      expose_buffer.push_back(filter_already_exposed(left_tile.down()));
      expose_buffer.push_back(filter_already_exposed(left_tile));
      expose_buffer.push_back(filter_already_exposed(left_tile.up()));
    }

    expose_buffer.push_back(
        filter_already_exposed(board_selection_optional.up()));

    {
      const auto right_tile = board_selection_optional.right();
      expose_buffer.push_back(filter_already_exposed(right_tile.up()));
      expose_buffer.push_back(filter_already_exposed(right_tile));
      expose_buffer.push_back(filter_already_exposed(right_tile.down()));
    }

    expose_buffer.push_back(
        filter_already_exposed(board_selection_optional.down()));
  }

  struct ExposeResultNext final : public ExposeResultContinuation {
    expose_result operator()() const override {
      static constexpr std::uint8_t MAX_EXPOSE_PER_ITER = 1;
      std::uint8_t exposed = 0;

      while (!expose_buffer.empty()) {

        const auto expose_target = expose_buffer.pop_back();
        const auto flagged = game_state.is_flagged(expose_target);

        if (game_state.count_mine(expose_target) && !flagged) {
          GameBoardDrawer::Mine(expose_target);
          return {false, nullptr};
        }

        if (game_state.exposed_test_and_set(expose_target)) {
          continue;
        }

        // bad-flag check.
        if (flagged) {
          if (!game_state.count_mine(expose_target)) {
            if (!bad_flag_at(expose_target))
            {
              return {false, nullptr};
            }
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

        if (mine_count == 0) {
          append_back_expose_buffer(expose_target);
          prev_exposed = expose_target;
        }

        // Early exit by limiting the max number of "expose" tiles that change in the current
        // invocation of this call.
        if (exposed == MAX_EXPOSE_PER_ITER) {
          // copy the un-traversed region of the front buffer to the back
          // buffer, so it will be examined at some point in the future.
          // for (const auto &unexposed_tile : *front_expose_buffer) {
          //   back_expose_buffer->push_back(unexposed_tile);
          // }

          break;
        }
      }

      return {true, expose_buffer.empty() ? nullptr : this};
    }
  };

  const ExposeResultNext expose_must_continue;

  expose_result
  expose_recurse(const TilePoint & board_selection, std::uint8_t depth) {

    const auto flagged = game_state.is_flagged(board_selection);
    if (game_state.count_mine(board_selection) && !flagged) {
      GameBoardDrawer::Mine(board_selection);
      return {false, nullptr};
    }

    if (flagged)
    {
      return {true, nullptr};
    }

    const auto already_exposed =
        game_state.exposed_test_and_set(board_selection);

    static constexpr uint8_t already_exposed_unhide_count[] = { 1, 0 };
    game_state.hidden_clear -= already_exposed_unhide_count[already_exposed];

    const auto mine_count = game_state.count_mines_around(board_selection);
    const auto flag_count = game_state.count_flags_around(board_selection);

    if (flag_count > mine_count) {
      return {bad_around_selection(board_selection), nullptr};
    }

    GameBoardDrawer::ShowCount(mine_count, board_selection);

    if (mine_count == 0 || (already_exposed && flag_count == mine_count)) {
      expose_buffer.clear();
      init_back_expose_buffer(board_selection);
      prev_exposed = board_selection;
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

    Event operator()(const key_scan_res current_state) {
      if (pressed) {
        if (current_state.space) {
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
        pressed = current_state.space;
        if (current_state.space) {
          return PRESS;
        }
      }

      // second button / 'b' button can only generate a 'long_press' event, on button up.aa
      if (secondary_pressed) {
        if (!current_state.fire_secondary) {
          secondary_pressed = false;
          return LONG_PRESS;
        }
      }
      else{
        secondary_pressed = current_state.fire_secondary;
      }

      return NO_EVENT;
    }

  private:
    bool pressed:1;
    bool secondary_pressed:1;
    std::uint8_t down_count;
  };

  class CursorAnimateFunc {
  public:
    void operator()() {
      static constexpr std::uint8_t cursor_frameskip = 1;
      static std::uint8_t cursor_current_frameskip = 0;

      if (cursor_current_frameskip++ == cursor_frameskip) {
        cursor_current_frameskip = 0;

        cursor_anim_frame += 1;
        if (cursor_anim_frame == target::graphics::Cursor.number_of_frames) {
          cursor_anim_frame = 0;
        }
      }

      cursor.select_frame(target::graphics::Cursor, cursor_anim_frame);
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

#ifdef PLATFORM_C64
    sprite_background.position(
        GameBoardDrawer::tile_to_sprite_x(GameBoardDrawer::reset_button_x()),
        GameBoardDrawer::tile_to_sprite_y(GameBoardDrawer::reset_button_y()));
#endif

    static constexpr std::uint8_t cursor_frameskip = 1;
    static std::uint8_t cursor_current_frameskip = 0;

    game_state.reset();

    for (std::uint8_t mines_left = mines; mines_left;) {
      const auto rownum = static_cast<std::uint8_t>(rand() >> 8) % game_rows;
      auto &row = game_state.mine_bits[rownum];
      const auto column = static_cast<std::uint8_t>(rand() >> 8) % game_columns;
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
    virtual void on_init(AppMode * last_mode) = 0;
  };

  struct AppModeGame final : public AppMode {

    void on_init(AppMode * last_mode) override;

    AppMode *on_vsync(FireButtonEventFilter::Event, key_scan_res) override;

    static bool continue_expose_events();

    static TilePoint current_selected;
    // Keeps track of cases where we are clearing a flag and don't want
    // to expose on fire-up.
    static bool suppress_expose;
  };

  TilePoint AppModeGame::current_selected{0, 0};
  bool AppModeGame::suppress_expose = false;

  struct AppModeResetButton : public AppMode {
    void on_init(AppMode *) override{}
    AppMode *on_vsync(FireButtonEventFilter::Event, key_scan_res) override;
  };

  struct AppModeSelectDifficulty : public AppMode {

    void on_init(AppMode *) override {
      static constexpr auto SELECT_DIFFICULTY =
          GameBoardDrawer::GenerateTileString("SELECT DIFFICULTY ");
      static constexpr auto BEGINNER = 
          GameBoardDrawer::GenerateTileString("BEGINNER");
      static constexpr auto INTERMEDIATE = 
          GameBoardDrawer::GenerateTileString("INTERMEDIATE");
      static constexpr auto EXPERT =
          GameBoardDrawer::GenerateTileString("EXPERT");
      target::graphics::render_off();
      target::clear_screen();
      target::graphics::load_pallettes(target::graphics::DifficultyScreenPallettes);
      GameBoardDrawer::DrawString(SELECT_DIFFICULTY, 1, 3);
      GameBoardDrawer::DrawString(BEGINNER, 5, 5);
      GameBoardDrawer::DrawString(INTERMEDIATE, 5, 7);
      GameBoardDrawer::DrawString(EXPERT, 5, 9);
      target::graphics::render_on();
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
    void on_init(AppMode *) override{}
    AppMode *on_vsync(FireButtonEventFilter::Event, key_scan_res) override;
  };

  struct AppModeWin : public AppMode {
    void on_init(AppMode *) override {}
    AppMode *on_vsync(FireButtonEventFilter::Event, key_scan_res) override;
  };

  AppModeGame game_field;

  AppModeResetButton reset_selection;
  AppModeDead mode_dead;
  AppModeWin mode_win;

  constexpr AppMode * next_mode_win[2] = { &game_field, &mode_win};

#ifdef PLATFORM_C64

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
#endif

  bool AppModeGame::continue_expose_events() {
    // "expose" means we are figuring out which tiles need to be automatically
    // opened up because there are no mines around them. Figuring out which
    // ones are like this can take a long time, so we split the operation up
    // into a sequence of function calls that expose only a few tiles per frame.
    // Each of these function object pointers is stored in
    // game_state.expose_continuation for the current frame.
    if (!game_state.expose_continuation) {
      return true;
    }

    // avoid virtual function call to work-around lack of stack on NES.
    // clang currently always requires the stack for that function call??
    if (game_state.expose_continuation == &expose_must_continue) {
      const auto expose_res =
          expose_must_continue(); //(*game_state.expose_continuation)();
      game_state.expose_continuation = expose_res.next_expose;
      if (!expose_res.is_ok) {
        return false;
      }
      suppress_expose = game_state.expose_continuation != nullptr;
      return true;
    }

    return true;  
  }

  AppMode *
  AppModeGame::on_vsync(FireButtonEventFilter::Event fire_button_events,
                        key_scan_res direction_events) {

    if (!continue_expose_events()) {
      game_state.time_running = false;
      return &mode_dead;
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
      if (!game_state.is_exposed(current_selected)) {
        if (game_state.set_flag(current_selected)) {
#ifdef PLATFORM_C64
          c64::MusicPlayer::play(0, false, flag_sfx);
#endif
          GameBoardDrawer::Flag(current_selected);
        }
        else {
          suppress_expose = true;
          GameBoardDrawer::Hide(current_selected);
        }
      }
    } break;
    case FireButtonEventFilter::PRESS:
#ifdef PLATFORM_C64
      c64::MusicPlayer::play(0, false, expose_sfx);
#endif
      GameBoardDrawer::DrawResetButtonCaution();
      break;
    case FireButtonEventFilter::NO_EVENT:
      break;
    }

    if (direction_events.w || direction_events.a || direction_events.s || direction_events.d) {
#ifdef PLATFORM_C64
      c64::MusicPlayer::play(1, false, shoot_sfx);
#endif
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
        target::graphics::render_off();
        reset();
        target::graphics::render_on();
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

      constexpr DifficultySettings(std::uint8_t rows, std::uint8_t columns,
                                   std::uint8_t mines)
          : m_rows{rows}, m_columns{columns}, m_mines{mines} {}
    };

    static constexpr DifficultySettings DIFFICULTY_PRESETS[] = {
        {9, 9, 10}, {16,16,40}, {16,30,99}
    };

    switch (fire_button_events) {
    case FireButtonEventFilter::RELEASE:
      GameBoardDrawer::SetGameSize(DIFFICULTY_PRESETS[difficulty].m_rows,
                                   DIFFICULTY_PRESETS[difficulty].m_columns);
      mines = DIFFICULTY_PRESETS[difficulty].m_mines;
      reset();
      target::graphics::render_off();
      game_field.on_init(this);
      target::graphics::render_on();
      return &game_field;
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
    
    GameBoardDrawer::Traits::place(target::graphics::BLANK, DifficultyToSelectionArrow[BEGINNER]);
    GameBoardDrawer::Traits::place(target::graphics::BLANK,
                                   DifficultyToSelectionArrow[INTERMEDIATE]);
    GameBoardDrawer::Traits::place(target::graphics::BLANK,
                                   DifficultyToSelectionArrow[EXPERT]);
    GameBoardDrawer::Traits::place(GameBoardDrawer::Traits::SelectArrow,
                                   DifficultyToSelectionArrow[difficulty]);
    return this;
  }

  AppModeSelectDifficulty difficulty_selection;

  void AppModeGame::on_init(AppMode *last_mode) {
    target::graphics::load_pallettes(target::graphics::GameBoardPallettes);

    if (last_mode == &difficulty_selection) {
      target::graphics::render_off();
      target::clear_screen();
      reset();
      target::graphics::render_on();
      current_selected = TilePoint{};
    }
  }

  AppMode *current_mode = &difficulty_selection;

  AppMode * AppModeDead::on_vsync(FireButtonEventFilter::Event fire_button_events, key_scan_res) {
    switch (fire_button_events) {
    case FireButtonEventFilter::RELEASE:
      difficulty_selection.on_init(this);
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
      difficulty_selection.on_init(this);
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

#ifdef PLATFORM_C64
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
#endif
  } // namespace

int main()
{
  if (!target::startup_check()) { return 1; }

  clock_updater.frames_per_second = target::frames_per_second();
  
  target::load_all_graphics();

  auto vsync_waiter = target::get_vsync_wait();

  target::graphics::render_off();
  target::clear_screen();

  // cursor shall be sprite 0.
  cursor.activate(0, target::graphics::Cursor, false);
  cursor.enable(true);
  cursor.multicolor_enable(false);

#ifdef PLATFORM_C64

  sprite_background.activate(7, target::graphics::SpriteBackground, true);
  sprite_background.enable(true);
  sprite_background.multicolor_enable(false);

#endif

  current_mode->on_init(nullptr);

  srand(target::seed_rng());

#ifdef PLATFORM_C64
  c64::sid.clear();
  c64::sid.set_volume(c64::Nibble{15}, c64::SID::VolumeBits{});
  c64::sid.voices[0].set_attack_decay(c64::Nibble{0}, c64::Nibble{9});
  c64::sid.voices[0].set_sustain_release(c64::Nibble{15}, c64::Nibble{1});
  c64::sid.voices[1].set_attack_decay(c64::Nibble{0}, c64::Nibble{6});
  c64::sid.voices[1].set_sustain_release(c64::Nibble{0}, c64::Nibble{0});
#endif

  while (true) {
    
    vsync_waiter();
    target::graphics::finish_rendering();

    rand();

    static key_scan_res keys;
    keys = target::check_keys();

    current_mode = current_mode->on_vsync(fire_button_handler(keys),
                                          direction_event_filter(keys));

    game_state.time_running && clock_updater();
#ifdef PLATFORM_C64
    c64::MusicPlayer::update();
#endif
  }

  return 0;
}
