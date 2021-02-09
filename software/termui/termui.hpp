/**
 *   Copyright 2021 Oleh Sharuda <oleh.sharuda@gmail.com>
 *
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

/*!  \file
 *   \brief TERMUI header
 *   \author Oleh Sharuda
 */

#pragma once

#include <ncursesw/curses.h>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <list>
#include <cassert>
#include "texttools.hpp"

/// \addtogroup group_tools
/// @{
/// \defgroup group_termui TermUI
/// \brief Simple ncurses based terminal TUI
/// @{
/// \page page_termui
/// \tableofcontents
///
/// \section sect_termui_01 Terminal UI
///
/// Terminal UI is a set of classes written especially for Home Lab Easy Kit testing purposes. The main idea of this
/// UI is to add ability to test software/hardware without dependency to XServer. Therefore it is ncurses based - it gives
/// possibility to use it through SSH or to run on old micro-computers without graphical subsystem configured.
///
/// It features simple windowing primitives that allow to implement basic functionalities like:
/// - Input window
/// - Read only text window (designed to store logs)
/// - List window
/// - Scrolling
///

/// \brief namespace for TERMUI
namespace TERMUI {

class TUI;
class TUWindow;
class TUTextWindow;
class TUInputWindow;
template<class T> class TUListWindow;

typedef int(*IntValFn)();
typedef std::vector<std::string> TUTextLines;
typedef std::vector<std::wstring> TUWTextLines;
typedef std::shared_ptr<TUWindow> TUWndPtr;
typedef std::map<int, TUWndPtr> TUWndMap;
typedef std::shared_ptr<TUTextWindow> TUTextWndPtr;
typedef std::shared_ptr<TUInputWindow> TUInputWndPtr;
template <typename T> 
using TUListWndPtr = std::shared_ptr<TUListWindow<T>>;


#define COLOR_WINDOW_BOX_COLOR          1
#define COLOR_ACTIVE_WINDOW_BOX_COLOR   2
#define COLOR_WINDOW_TITLE              3
#define COLOR_ACTIVE_WINDOW_TITLE       4
#define COLOR_TEXT                      5
#define COLOR_SELECTION                 6

#define TITLE_OFFSET_RIGHT              -1
#define TITLE_OFFSET_LEFT               -2
#define TITLE_OFFSET_MIDDLE             -3

/// \def SCROLLBAR_OFF
/// \brief Disables scrollbar
#define SCROLLBAR_OFF	0

/// \def SCROLLBAR_AUTO
/// \brief Scrollbar will hide if not required.
#define SCROLLBAR_AUTO	1

/// \def SCROLLBAR_ON
/// \brief Scrollbar is enabled.
#define SCROLLBAR_ON	2

/// \class TUPoint
/// \brief Point implementation
class TUPoint {
protected:	
	int x0; ///< Point X coordinate
	int y0; ///< Point Y coordinate
public:
    /// \brief Returns point X coordinate
    /// \return point x coordinate
	int x() const;

    /// \brief Returns point Y coordinate
    /// \return point y coordinate
	int y() const;

	/// \brief Default constructor
	TUPoint();

	/// \brief Constructor with coordinates specified explicitly.
	/// \param _x - x coordinate
	/// \param _y - y coordinate
	TUPoint(int _x, int _y);

	/// \brief Destructor (virtual)
	virtual ~TUPoint();
};

/// \class TURect
/// \brief Rectangle implementation
class TURect : public TUPoint{

    /// \typedef super
    /// \brief Typedef for parent class.
    /// \details Parent class represents point that correspond to the left upper corner of the rectangle
	typedef TUPoint super;

	/// \brief Updates right upper, left and right bottom corners
    void update_corners();

    int w0; ///< Width of the rectangle
    int h0; ///< Height of the rectangle

    TUPoint ruc0;   ///< Coordinates of the right upper corner
    TUPoint lbc0;   ///< Coordinates of the left bottom corner
    TUPoint rbc0;   ///< Coordinates of the right bottom corner

public:
    /// \brief Returns width of the rectangle
    /// \return width
	int w() const;

    /// \brief Returns height of the rectangle
    /// \return height
	int h() const;

    /// \brief Default constructor
	TURect();

    /// \brief Constructor with coordinates specified explicitly.
    /// \param _x - x coordinate
    /// \param _y - y coordinate
    /// \param _w - width
    /// \param _h - height
	TURect(int _x, int _y, int _w, int _h);

    /// \brief Destructor (virtual)
	~TURect() override;

	/// \brief Transforms rectangle by changing coordinates, width and height by specified deltas
	/// \param _x - change of the left upper corner X coordinate (may be negative)
	/// \param _y - change of the left upper corner Y coordinate (may be negative)
	/// \param _w - change of the width (may be negative)
	/// \param _h - change of the height (may be negative)
	void transform(int _x, int _y, int _w, int _h);

	/// \brief Transforms rectangle by calling unary functions which returns new values
	/// \param x_fn Unary function returning X coordinate
	/// \param y_fn Unary function returning Y coordinate
	/// \param w_fn Unary function returning width coordinate
	/// \param h_fn Unary function returning hight coordinate
	/// \return true if geometery has changed, otherwise false
	bool transform(IntValFn x_fn, IntValFn y_fn, IntValFn w_fn, IntValFn h_fn);

	/// \brief Returns left upper corner
	/// \return left upper corner coordinates
	const TUPoint& luc() const;

    /// \brief Returns right upper corner
    /// \return right upper corner coordinates
	const TUPoint& ruc() const;

    /// \brief Returns left bottom corner
    /// \return  left bottom corner coordinates
	const TUPoint& lbc() const;

    /// \brief Returns right bottom corner
    /// \return  right upper bottom coordinates
	const TUPoint& rbc() const;

	/// \brief Check if coordinates belong this rectangle (edges belong to the rectangle)
	/// \param _x - x coordinate
	/// \param _y - y coordinate
	/// \return returns true if coordinates belong to the rectangle
	bool belong(int _x, int _y) const;

	/// \brief Check if rectangle is drawable inside another rectangle
	/// \param outer - outer rectangle
	/// \return true if *this rectangle is drawable inside outer (edges may overlap), otherwise false
	bool drawable(const TURect& outer) const;

	/// \brief Returns coordinates of the left upper corner, width and height.
	/// \param _x - reference for output value for X coordinate
	/// \param _y - reference for output value for Y coordinate
	/// \param _w - reference for output value for W coordinate
	/// \param _h - reference for output value for H coordinate
	void get(int& _x, int& _y, int& _w, int& _h) const;
};

/// \class TUWindow
/// \brief Window implementation
/// \details Windows consist of window area and a canvas - area dedicated for informational purposes. Canvas is inside
///          window area.
class TUWindow {
    friend TUI;

    TURect winrect;     ///< window area rectnagle

    TURect canvrect;    ///< canvas area rectangle

    std::shared_ptr<TUI> ui;    ///< shared pointer for the main ui

    IntValFn x_fn;              ///< unary function to get window X coordinate (left upper corner)

    IntValFn y_fn;              ///< unary function to get window Y coordinate (left upper corner)

    IntValFn w_fn;              ///< unary function to get window width

    IntValFn h_fn;              ///< unary function to get window height

    bool boxed = false;         ///< true if window has a frame (box) on it's edges

    std::string title;          ///< title of the window

    int title_offset;           ///< offset of this title (may be #TITLE_OFFSET_LEFT, #TITLE_OFFSET_RIGHT or #TITLE_OFFSET_MIDDLE)

    bool active=false;          ///< true if window is active

    int index = -1;             ///< index of the window

    int wnd_color;              ///< window color pairs (see ncurses documentation for init_pair())

    /// \brief destroys window
    void destroy();

    /// \brief clears canvas area
    void clear_canvas();

protected:
    WINDOW* wnd = nullptr;  ///< ncurses window

    /// \brief Updates internal canvas (virtual)
    virtual void update_canvas();

public:
    TUWindow() = delete;                            ///< No default constructor
    TUWindow(const TUWindow&) = delete;             ///< This class doesn't allow copying
    TUWindow& operator()(const TUWindow&) = delete; ///< This class doesn't allow assignment

    /// \brief TUWindow class constructor
    /// \param _x - Unary function to get X coordinate
    /// \param _y - Unary function to get Y coordinate
    /// \param _w - Unary function to get width
    /// \param _h - Unary function to get height
    TUWindow (IntValFn _x, IntValFn _y, IntValFn _w, IntValFn _h);

    /// \brief Destructor (virtual)
    virtual ~TUWindow();

    /// \brief Virtual function that redraws window
    /// \return true if windows is drawable and was redrawn. Otherwise false.
    virtual bool redraw();

    /// \brief Sets frame (box) on window
    /// \param b - true if box is required, otherwise false
    /// \param t - title of the window
    /// \param offset - offset of the title (may be #TITLE_OFFSET_LEFT, #TITLE_OFFSET_RIGHT or #TITLE_OFFSET_MIDDLE)
    void set_box(bool b, const std::string& t, int offset);

    /// \brief Sets window colors
    /// \param cp - window color pairs (see ncurses documentation for init_pair())
    void set_color(int cp);

    /// \brief Establishes link with main UI
    /// \param _ui - shared pointer to the man UI object
    void set_ui(std::shared_ptr<TUI>& _ui);

    /// \brief Checks if window is active
    /// \return true if window is active, otherwise false
    bool is_active() const;

    /// \brief Returns window index
    /// \return window index
    /// \details Each window should have a unique index, that represents it. These indexes are used in message handling.
    int get_index() const;

    /// \brief Set window index
    /// \param indx - window index
    /// \details Each window should have a unique index, that represents it. These indexes are used in message handling.
    void set_index(int indx);

    /// \brief Put text into window
    /// \param cx - X coordinate (relative to window canvas area)
    /// \param cy - Y coordinate (relative to window canvas area)
    /// \param ws - string to be put
    void put_text(int cx, int cy, const std::wstring& ws);

    /// \brief Put cursor into window
    /// \param cx - X coordinate (relative to window canvas area)
    /// \param cy - Y coordinate (relative to window canvas area)
    void put_cursor(int cx, int cy);

    /// \brief Put vertical line on the window
    /// \param cx - X coordinate (relative to window canvas area)
    /// \param cy - Y coordinate (relative to window canvas area)
    /// \param len - length of the line
    /// \param wc - character representing a line
    /// \param cp - window color pairs (see ncurses documentation for init_pair())
    /// \details This function doesn't check if line goes out of the window canvas area
    void put_vert_line(int cx, int cy, int len, wchar_t wc, int cp);

    /// \brief Put horizontal line on the window
    /// \param cx - X coordinate (relative to window canvas area)
    /// \param cy - Y coordinate (relative to window canvas area)
    /// \param len - length of the line
    /// \param wc - character representing a line
    /// \param cp - window color pairs (see ncurses documentation for init_pair())
    /// \details This function doesn't check if line goes out of the window canvas area
    void put_horiz_line(int cx, int cy, int len, wchar_t wc, int cp);

    /// \brief Set's window as active (virtual)
    /// \param a - true if window is active, otherwise false
    virtual void set_active(bool a);

    /// \brief Window class message handler
    /// \param ch - character
    /// \param err - error code
    /// \return true if message was handled, otherwise false
    /// \details Parameters passed into this function are obtained from wget_wch(). For more details see ncurses documentation.
    virtual bool handler(wint_t ch, int err);

    /// \brief Convert point X coordinate in window canvas area to window area
    /// \param cx - X coordinate (relative to window canvas area)
    /// \return X coordinate (relative to window area)
    virtual int to_wnd_x(int cx) const;

    /// \brief Convert point Y coordinate in window canvas area to window area
    /// \param cy - Y coordinate (relative to window canvas area)
    /// \return Y coordinate (relative to window area)
    virtual int to_wnd_y(int cy) const;

    /// \brief Returns constant reference to window canvas area
    /// \return constant reference to window canvas area (relative to window area)
    virtual const TURect& canvas() const;

    /// \brief Check if window is drawable
    /// \return true if window is drawable, otherwise false
    virtual bool drawable() const;
};

/// \class TUScrollWindow
/// \brief Interface for scrollable window (partially implemented)
class TUScrollWindow : public TUWindow {

    /// \typedef super
    /// \brief Typedef for parent class.
	typedef TUWindow super;

    /// \brief Area of the data represented by derived class
    TURect data_area;

	/// \brief Visible area that will be drawn in the window.
	TURect scroll_area;

    /// \brief Area of the window, excluding X and Y scrollbars.
    TURect scrollcanv;

    /// \brief Area occupied by X scroll bar
    TURect x_scroll_area;

    /// \brief Area occupied by Y scroll bar
    TURect y_scroll_area;

    /// \brief X scroll bar stored parameter in context of TUScrollWindow#set_scroll_bar()
    int scroll_bar_x;

    /// \brief Y scroll bar stored parameter in context of TUScrollWindow#set_scroll_bar()
    int scroll_bar_y;

    /// \brief true of X scrollbar is visible.
    bool draw_scroll_bar_x = false;

    /// \brief true of Y scrollbar is visible.
    bool draw_scroll_bar_y = false;

    /// \brief Used to correct values used in TUScrollWindow#set_scroll_area().
    void correct_scroll_area();

protected:

    /// \brief Updates internal canvas (virtual)
    void update_canvas() override;

    /// \brief Draws scroll bars
    virtual void draw_scrolls();

public:

    TUScrollWindow() = delete;                                  ///< No default constructor
    TUScrollWindow(const TUScrollWindow&) = delete;             ///< This class doesn't allow copying
    TUScrollWindow& operator()(const TUScrollWindow&) = delete; ///< This class doesn't allow assignment

    /// \brief TUWindow class constructor
    /// \param _x - Unary function to get X coordinate
    /// \param _y - Unary function to get Y coordinate
    /// \param _w - Unary function to get width
    /// \param _h - Unary function to get height
    TUScrollWindow (IntValFn _x, IntValFn _y, IntValFn _w, IntValFn _h);

    /// \brief Destructor (virtual)
    ~TUScrollWindow() override;

    /// \brief Updates scrolling position
    /// \param x - new X coordinate of the left upper corner of scrolling area.
    /// \param y - new Y coordinate of the left upper corner of scrolling area.
    /// \param relative - if true, x and y are deltas to change current coordinates of the left upper corner.
    /// \return updated rectangle that corresponds to the visible area
    virtual TURect set_scroll_area(int x, int y, bool relative);

    /// \brief Enables or disables scroll bars
    /// \param xbar - X scrollbar. Must be one of the following: #SCROLLBAR_OFF, #SCROLLBAR_AUTO, #SCROLLBAR_ON.
    /// \param ybar - Y scrollbar. Must be one of the following: #SCROLLBAR_OFF, #SCROLLBAR_AUTO, #SCROLLBAR_ON.
    void set_scroll_bar(int xbar, int ybar);

    /// \brief Pure virtual function to be implemented by derived class. Must return size of the canvas that represent data available.
    /// \return Size of the data in the window.
    virtual TURect data_dim() const = 0;

    /// \brief Pure virtual function to be implemented by derived class. Must redraw data in window
    /// \param area - rectangle that represents area that should be actually put on the screen.
    /// \return true if area was drawn, otherwise false.
    virtual bool redraw_area(const TURect& area) = 0;

    /// \brief Scrollable window class message handler
    /// \param ch - character
    /// \param err - error code
    /// \return true if message was handled, otherwise false
    /// \details Parameters passed into this function are obtained from wget_wch(). For more details see ncurses documentation.
    bool handler(wint_t ch, int err) override;

    /// \brief Virtual function that redraws window
    /// \return true if windows is drawable and was redrawn. Otherwise false.
    bool redraw() override;

    /// \brief Check if window is drawable
    /// \return true if window is drawable, otherwise false
    bool drawable() const override;

    /// \brief Convert point X coordinate in window canvas area to window area
    /// \param cx - X coordinate (relative to window canvas area)
    /// \return X coordinate (relative to window area)
    int to_wnd_x(int cx) const override;

    /// \brief Convert point Y coordinate in window canvas area to window area
    /// \param cy - Y coordinate (relative to window canvas area)
    /// \return Y coordinate (relative to window area)
    int to_wnd_y(int cy) const override;

    /// \brief Returns constant reference to window canvas area
    /// \return constant reference to window canvas area (relative to window area)
    const TURect& canvas() const override;
};

/// \class TUTextWindow
/// \brief Text window (scrollable) implementation
class TUTextWindow : public TUScrollWindow {

    /// \typedef super
    /// \brief Typedef for parent class.
    typedef TUScrollWindow super;

    TUWTextLines text;          ///< Window text container (vector of wide character strings)
    bool auto_scroll = true;    ///< true if any text change should scroll text down. If false scrolling won't happen.

    /// \brief Updates text on the window
    /// \param scroll_down - if true - window will be scrolled down, if false window will not be scrolled.
    void update_window(bool scroll_down);

public:

    TUTextWindow() = delete;                                ///< No default constructor
    TUTextWindow(const TUTextWindow &) = delete;             ///< This class doesn't allow copying
    TUTextWindow &operator()(const TUTextWindow &) = delete; ///< This class doesn't allow assignment

    /// \brief TUTextWindow class constructor
    /// \param _x - Unary function to get X coordinate
    /// \param _y - Unary function to get Y coordinate
    /// \param _w - Unary function to get width
    /// \param _h - Unary function to get height
    TUTextWindow(IntValFn _x, IntValFn _y, IntValFn _w, IntValFn _h);

    /// \brief Destructor (virtual)
    ~TUTextWindow() override;

    /// \brief Appends text
    /// \param t - text to append in a form of vector of strings
    void append(const TUTextLines &t);

    /// \brief Appends text
    /// \param l - text to append in a form of string.
    /// \details line is split by new line character
    void append(const std::string &l);

    /// \brief Clears text
    void clear();

    /// \brief Set text
    /// \param t - text to set in a form of vector of strings
    void set_text(const TUTextLines &t);

    /// \brief TUTextWindow window class message handler
    /// \param ch - character
    /// \param err - error code
    /// \return true if message was handled, otherwise false
    /// \details Parameters passed into this function are obtained from wget_wch(). For more details see ncurses documentation.
    bool handler(wint_t ch, int err) override;

    /// \brief Virtual function to be called by parent class. Must redraw data in visible window area.
    /// \param area - rectangle that represents area that should be actually put on the screen.
    /// \return true if area was drawn, otherwise false.
    bool redraw_area(const TURect &area) override;

    /// \brief Virtual function to be called by parent class. Must return size of the canvas that represent data available.
    /// \return Size of the data in the window.
    TURect data_dim() const override;
};

/// \class TUInputWindow
/// \brief Single line input window (scrollable) implementation
class TUInputWindow : public TUScrollWindow {

    /// \typedef super
    /// \brief Typedef for parent class.
    typedef TUScrollWindow super;

    std::wstring text;          ///< Input string container
    int cur_pos = 0;            ///< Cursor position
    int min_cur_pos = 0;        ///< Minimum cursor position
    int max_cur_pos = 0;        ///< Maximum cursor position
    bool insert_mode = true;    ///< If true insert mode is on, otherwise overwrite mode.

    /// \brief Handles input character
    /// \param ch - character to handle
    void handle_char(wint_t ch);

    /// \brief Handles cursor movement
    /// \param pos_change - incremental change of cursor position (may be negative)
    /// \param absolute - if true pos_change - is absolute cursor position, otherwise incremental value.
    void handle_cursor(int pos_change, bool absolute);

	public:
    TUInputWindow() = delete;                                  ///< No default constructor
    TUInputWindow(const TUInputWindow &) = delete;             ///< This class doesn't allow copying
    TUInputWindow &operator()(const TUInputWindow &) = delete; ///< This class doesn't allow assignment

    /// \brief TUInputWindow class constructor
    /// \param _x - Unary function to get X coordinate
    /// \param _y - Unary function to get Y coordinate
    /// \param _w - Unary function to get width
    /// \param _h - Unary function to get height
	TUInputWindow(IntValFn _x, IntValFn _y, IntValFn _w, IntValFn _h);

    /// \brief Destructor (virtual)
	~TUInputWindow() override;

    /// \brief Set's window as active (virtual)
    /// \param a - true if window is active, otherwise false
	void set_active(bool a) override;

    /// \brief TUTextWindow window class message handler
    /// \param ch - character
    /// \param err - error code
    /// \return true if message was handled, otherwise false
    /// \details Parameters passed into this function are obtained from wget_wch(). For more details see ncurses documentation.
    bool handler(wint_t ch, int err) override;

    /// \brief Set text
    /// \param t - text string to be set
    void set_text(const std::string&);

    /// \brief Returns text in the window
    /// \return text in window
    std::string get_text();

    /// \brief Set insert mode
    /// \param mode - If true insert mode is on, otherwise overwrite mode.
    void set_insert_mode(bool mode);

    /// \brief Virtual function to be called by parent class. Must return size of the canvas that represent data available.
    /// \return Size of the data in the window.
	TURect data_dim() const override;

    /// \brief Virtual function to be called by parent class. Must redraw data in visible window area.
    /// \param area - Rectangle that represents area that should be actually put on the screen.
    /// \return true if area was drawn, otherwise false.
	bool redraw_area(const TURect& area) override;
};

/// \class TUListItem
/// \brief List item class (template) for @ref TUListWindow
template<class T>
class TUListItem {
	friend TUListWindow<T>;

protected:

	int index;          ///< List item index

	std::wstring name;  ///< List item name (wide character string)

	T value;            ///< List item value

public:
    /// \brief TUListItem constructor
    /// \param _index - List item index
    /// \param _name - List item name
    /// \param _value - List item value
	TUListItem(int _index, const std::string& _name, T& _value) : index(_index), value(_value){
		set_name(_name);
	}

    /// \brief Destructor (virtual)
	~TUListItem(){};

    /// \brief Returns list item index
    /// \return List item index
	int get_index() const {return index;}

	/// \brief Sets list item index
	/// \param _index - List item index
	void set_index(int _index) {index = _index;}

	/// \brief Returns list item name
	/// \return List item name
	std::string get_name() const {return tools::wstr_to_utf8(name);}

	/// \brief Returns list item name (wide character string)
	/// \return List item name (wide character string)
	const std::wstring& get_wname() const {return name;}

	/// \brief Sets list item name
	/// \param _name - List item name
	void set_name(const std::string& _name) {name = tools::utf8_to_wstr(_name);}

	/// \brief Returns reference to the list item value
	/// \return Reference to the list item value
	T& get() { return value; }
};

/// \class TUListWindow
/// \brief Scrollable item list implementation (template)
template<class T>
class TUListWindow : public TUScrollWindow {

    /// \typedef super
    /// \brief Typedef for parent class.
    typedef TUScrollWindow super;

    std::list<TUListItem<T>> items; ///< List of the items that belong to this window

    int cur_pos         = 0;        ///< Cursor position
    int min_cur_pos     = 0;        ///< Minimal cursor position
    int max_cur_pos     = 0;        ///< Maximal cursor position
    bool item_search    = true;     ///< If true - items may be selected from keyboard using first name letters

	public:

    TUListWindow() = delete;                                 ///< No default constructor
    TUListWindow(const TUListWindow &) = delete;             ///< This class doesn't allow copying
    TUListWindow &operator()(const TUListWindow &) = delete; ///< This class doesn't allow assignment

    /// \brief TUListWindow class constructor
    /// \param _x - Unary function to get X coordinate
    /// \param _y - Unary function to get Y coordinate
    /// \param _w - Unary function to get width
    /// \param _h - Unary function to get height
	TUListWindow(IntValFn _x, IntValFn _y, IntValFn _w, IntValFn _h) : TUScrollWindow(_x, _y, _w, _h) {
        set_color(COLOR_TEXT);
	}

    /// \brief Destructor (virtual).
	~TUListWindow() override {}

	/// \brief Sets item search mode.
	/// \param on - If true, search mode is on, otherwise false.
	void set_item_search(bool on) {
        item_search = on;
	}

	/// \brief Insert item into the list
	/// \param pos - position of the item
	/// \param v - item value
	void insert(size_t pos, const T&& v) {
		typename std::list<TUListItem<T>>::iterator i = find_by_pos(pos);
		items.insert( i, v);
		reset_selection();
	}

	/// \brief Checks if list is empty.
	/// \return true if empty, otherwise false.
	bool empty() {
		return items.empty();
	}

	/// \brief Adds entry to the end of the list
	/// \param v - rvalue reference to the value. Note, v may be moved.
	void push_back(TUListItem<T>&& v) {
		items.push_back(std::move(v));
		reset_selection();
	}

	/// \brief Removes item from the list
	/// \param pos - position of the item to remove
	void remove(size_t pos) {
		typename std::list<TUListItem<T>>::iterator i = find_by_pos(pos);
		if (i==items.end()) {
			throw std::out_of_range("remove position doesn't correspond list content");
		}
		items.erase(i);
		reset_selection();
	}

	/// \brief Clears the list
	void clear() {
		items.clear();
		reset_selection();
	}

	/// \brief Select list item
	/// \return Reference to the value found
	T& sel_item() {
		return find_by_pos(cur_pos)->get();
	}

	/// \brief Returns currently selected position
	/// \return Currently selected position
	int sel_pos() const {
		return cur_pos;
	}

	/// \brief Sorts list items by name
	/// \param ascending - true if ascending sort order, false for descending sort order.
	void sort_by_name(bool ascending) {
		std::sort(	items.begin(), 
					items.end(), 
					[&](const TUListItem<T>& l, const TUListItem<T>& r)->bool{ return ascending ? l.name.compare(r.name) : r.name.compare(l.name);});
		reset_selection();
	}

	/// \brief Sort by index value
    /// \param ascending - true if ascending sort order, false for descending sort order.
	void sort_by_index(bool ascending) {
		std::sort(	items.begin(), 
					items.end(), 
					[&](const TUListItem<T>& l, const TUListItem<T>& r)->bool{ return ascending ? (l.get_index() < r.get_index()) : (r.get_index() <= l.get_index());});
		reset_selection();
	}

    /// \brief TUTextWindow window class message handler
    /// \param ch - character
    /// \param err - error code
    /// \return true if message was handled, otherwise false
    /// \details Parameters passed into this function are obtained from wget_wch(). For more details see ncurses documentation.
    bool handler(wint_t ch, int err) override {
	    bool res = false;
	    const TURect& canv = canvas();
	    int nitems = items.size();

	    if (err==KEY_CODE_YES) {
	        // functional key
	        res = true;
	        switch (ch) {
	            case KEY_UP:
	                handle_cursor(-1, false);
	            break;
	            case KEY_DOWN:
	                handle_cursor(1, false);
	            break;  
	            case KEY_HOME:
	                handle_cursor(0, true);
	            break;
	            case KEY_END:
	                handle_cursor(nitems, true);
	            break;
	            case KEY_PPAGE:
	                handle_cursor(-canv.h(), false);
	            break;
	            case KEY_NPAGE:
	                handle_cursor(canv.h(), false);
	            break;            

	            default:
	                res = false;
	        }
	    } else if (item_search && err==OK && ch!=L'\t' && ch!='\n') {
	        int nxt = find_next_by_wchar(ch);
	        if (nxt!=-1) {
	        	handle_cursor(nxt, true);
	        	res = true;	
	        }
	    }

	    // allow parent window to process scrolling and etc.
	    if (!res) {
	    	res |= super::handler(ch, err);	
	    }

	    return res;
    }

    /// \brief Virtual function to be called by parent class. Must return size of the canvas that represent data available.
    /// \return Size of the data in the window.
	TURect data_dim() const override {
		int len = items.size();
		int max_w = 0;
		for (typename std::list<TUListItem<T>>::const_iterator i=items.begin(); i != items.end(); ++i) {
			max_w = std::max(max_w, (int)i->name.length());
		}
		return TURect(0, 0, max_w, len);
	}

    /// \brief Virtual function to be called by parent class. Must redraw data in visible window area.
    /// \param area - Rectangle that represents area that should be actually put on the screen.
    /// \return true if area was drawn, otherwise false.
	bool redraw_area(const TURect& area) override {
	    int x,y,w,h;
        area.get(x, y, w, h);
	    int nitems = items.size();

	    min_cur_pos = y;
	    max_cur_pos = std::min(y + h, nitems) - 1;
   		
	    assert(min_cur_pos<=nitems);
	    assert(max_cur_pos<=nitems);

		typename std::list<TUListItem<T>>::iterator li = items.begin();
		std::advance(li, min_cur_pos);

	    for (int i = min_cur_pos; i<=max_cur_pos; ++i, ++li) {
	    	const std::wstring& wname = li->get_wname();
	    	int ilen = wname.length();
	    	if (ilen>x) {
		        std::wstring wss = wname.substr(x, w);
		        wbkgdset(wnd, COLOR_PAIR(i==cur_pos ? COLOR_SELECTION : COLOR_TEXT));
		        put_text(0,i-min_cur_pos,wss);
	    	}
	    }

    	return true;
	};	

	/// \brief Search list item by position
	/// \param pos - position of the list item
	/// \return - iterator to the corresponding TUListItem object.
	typename std::list<TUListItem<T>>::iterator find_by_pos(size_t pos) {
		size_t item_count = items.size();
		if (pos>=item_count) {
			return items.end();
		} else {
			typename std::list<TUListItem<T>>::iterator iter = items.begin();
			for (size_t i = 0; i<pos; ++i) {
				++iter;
			}
			return iter;
		}
	}

	/// \brief Search list item by the first letter
	/// \param wc - first letter to search list item.
	/// \return - position of the found list item
	int find_next_by_wchar(wchar_t wc) {
		typename std::list<TUListItem<T>>::const_iterator start = find_by_pos(cur_pos);
		typename std::list<TUListItem<T>>::const_iterator i = start;
		i++;
		int new_pos = cur_pos+1;

		do {
			if (i==items.end()) {
				i = items.begin();
				new_pos = 0;
				continue;
			}

			const std::wstring& in = i->get_wname();
			if (in.length() && in[0]==wc) {
				return new_pos;
			}

			new_pos++;
			i++;
		} while (i!=start);

		return cur_pos;
	}

private:

    /// \brief Reset selection
    void reset_selection() {
        cur_pos = (items.size()==0) ? -1 : 0;
    }

    /// \brief Handles cursor movement
    /// \param pos_change - incremental change of cursor position (may be negative)
    /// \param absolute - if true pos_change - is absolute cursor position, otherwise incremental value.
    void handle_cursor(int pos_change, bool absolute) {
        int nitems = items.size();
        int scroll_by_y = 0;

        if (nitems==0) {
            cur_pos = 0;
            min_cur_pos = 0;
            max_cur_pos = 0;
            return;
        }

        if (absolute) {
            cur_pos=pos_change;
        } else {
            cur_pos+=pos_change;
        }

        if (cur_pos<0) {
            cur_pos=0;
        } else
        if (cur_pos>=nitems) {
            cur_pos = nitems-1;
        }

        if (cur_pos<min_cur_pos) {
            // scroll up
            scroll_by_y = cur_pos - min_cur_pos;
        } else if (cur_pos>max_cur_pos) {
            // scroll down
            scroll_by_y = cur_pos - max_cur_pos;
        }

        if (scroll_by_y) {
            TURect new_area = set_scroll_area(0, scroll_by_y, true);
            min_cur_pos = new_area.y();
            max_cur_pos = std::min(min_cur_pos+new_area.h(), nitems)-1;
            assert(cur_pos>=min_cur_pos);
            assert(cur_pos<=max_cur_pos);
            assert(max_cur_pos<nitems);
        }
    }

};
/// \class TUI
/// \brief Terminal UI implementation
class TUI {
    TUWndMap windows;                   ///< Window index to TUWndPtr map
    int active_window = -1;             ///< Index of active window. Negative value means no currently active windows.
    TURect screen;                      ///< Rectangle that correspond to the terminal screen size.
    std::shared_ptr<TUI> this_shared;   ///< Shared pointer to the this object. It is used by the TUWindow classes.

    /// \brief Update screen size
    void update_screen();

public:

    TUI(const TUI &) = delete;             ///< This class doesn't allow copying
    TUI &operator()(const TUI &) = delete; ///< This class doesn't allow assignment

    /// \brief TUI constructor.
    TUI();

    /// \brief Destructor (virtual)
    virtual ~TUI();

    /// \brief Adds window to the UI
    /// \param index - Windown index (must be positive)
    /// \param wnd - Window shared pointer. Window must not be added to UI before.
    void add_window(size_t index, TUWndPtr wnd);

    /// \brief Initialize colors.
    void init_colors();

    /// \brief Set active window
    /// \param index - index of the window
    void set_active_window(int index);

    /// \brief Redraws UI
    /// \param all - true to redraw all windows, false to redraw currently active window.
    void redraw(bool all);

    /// \brief Returns window next to the window specified by index.
    /// \param index - window whose sibling is required.
    /// \return Index of the next window.
    int get_next_window(int index);

    /// \brief Returns window previous to the window specified by index.
    /// \param index - window whose sibling is required.
    /// \return Index of the previous window.
    int get_prev_window(int index);

    /// \brief TermUI message handler. May be overloaded by derived class in order to change logic of UI.
    /// \param index - index of the active window
    /// \param ch - character
    /// \param err - error code
    /// \details Parameters passed into this function are obtained from wget_wch(). For more details see ncurses documentation.
    virtual void message_handler(int index, wint_t ch, int err);

    /// \brief Message loop of the UI
    void runloop();

    /// \brief Returns const reference to rectangle that correspond to the terminal screen size.
    /// \return Const reference to rectangle that correspond to the terminal screen size.
    const TURect& scr() const{
    	return screen;
    }
};

}


/// @}
/// @}