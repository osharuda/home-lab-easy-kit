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
 *   \brief TERMUI implementation
 *   \author Oleh Sharuda
 */

#include "termui.hpp"
#include <string>
#include <cassert>
#include "tools.hpp"
#include "texttools.hpp"

using namespace TERMUI;

// ********************************** TUPoint *********************************
// Objective: base class for a point
//*****************************************************************************
TUPoint::TUPoint() : x0(-1), y0(-1) {
}

TUPoint::TUPoint(int _x, int _y) : x0(_x), y0(_y) {
}

TUPoint::~TUPoint() {
}

int TUPoint::x() const {
    return x0;
}

int TUPoint::y() const {
    return y0;
}

// ********************************** TURect **********************************
// Objective: represetns some rectanular area, with some helper things.
//*****************************************************************************
TURect::TURect() : super(), w0(-1), h0(-1) {
    update_corners();
}

TURect::TURect(int _x, int _y, int _w, int _h) : super(_x, _y), w0(_w), h0(_h) {
    update_corners();    
}

TURect::~TURect() {
}

int TURect::w() const {
    return w0;
}
int TURect::h() const {
    return h0;
}

void TURect::get(int& _x, int& _y, int& _w, int& _h) const {
    _x = x0;
    _y = y0;
    _w = w0;
    _h = h0;
}

bool TURect::belong(int _x, int _y) const {
    return _x>=x0 && _x<=(x0+w0-1) && _y>=y0 && _y<=(y0+h0-1);
}

bool TURect::drawable(const TURect& outer) const {
    return (x0>=outer.x0) && (y0>=outer.y0) && (x0+w0<=outer.w0) && (y0+h0<=outer.h0);
}

bool TURect::transform(IntValFn x_fn, IntValFn y_fn, IntValFn w_fn, IntValFn h_fn) {
    int nx = x_fn();
    int ny = y_fn();
    int nw = w_fn();
    int nh = h_fn();

    bool changed = (nx!=x0) || (ny!=y0) || (nw!=w0) || (nh!=h0);

    x0 = nx;
    y0 = ny;
    w0 = nw;
    h0 = nh;

    update_corners();

    return changed;
}

void TURect::update_corners() {
    ruc0 = TUPoint(x0 + w0 - 1, y0);
    lbc0 = TUPoint(x0, y0 + h0 - 1);
    rbc0 = TUPoint(x0 + w0 - 1, y0 + h0 - 1);
}

const TUPoint& TURect::luc() const {
    return static_cast<const TUPoint&>(*this);
}
const TUPoint& TURect::ruc() const {
    return ruc0;
}
const TUPoint& TURect::lbc() const {
    return lbc0;
}
const TUPoint& TURect::rbc() const {
    return rbc0;
}

void TURect::transform(int _x, int _y, int _w, int _h) {
    x0+=_x;
    y0+=_y;
    w0+=_w;
    h0+=_h;
    update_corners();
}

// ********************************** TUWindow **********************************
// Objective: Base window object.
//*****************************************************************************

TUWindow::TUWindow (IntValFn _x, IntValFn _y, IntValFn _w, IntValFn _h) : x_fn(_x), y_fn(_y), w_fn(_w), h_fn(_h) {
}

TUWindow::~TUWindow() {
    destroy();
}

void TUWindow::set_ui(std::shared_ptr<TUI>& _ui) {
    ui = _ui;
}

void TUWindow::update_canvas() {
    int x,y,w,h;
    winrect.get(x, y, w, h);
    canvrect = TURect(0, 0, w, h);
    if (boxed) {
        canvrect.transform(1,1,-2,-2);
    }
}

bool TUWindow::drawable() const {
    bool res = winrect.drawable(ui->scr());
    if (res && boxed) {
        res = res && canvrect.w()>0 && canvrect.h()>0;
    }
    return res;
}

void TUWindow::clear_canvas() {
    wbkgdset(wnd, COLOR_PAIR(wnd_color));
    werase(wnd);
}

bool TUWindow::redraw() {

    bool changed = winrect.transform(x_fn, y_fn, w_fn, h_fn);
    update_canvas();

    if (!drawable()) {
        return false;
    }

    if (changed) {
        if (wnd) {
            destroy();
        }
        wnd = newwin(winrect.h(), winrect.w(), winrect.y(), winrect.x());

        keypad(wnd, true);
    }

    clear_canvas();

    // draw title only if boxed
    if (boxed) {
        wbkgdset(wnd, COLOR_PAIR(active ? COLOR_ACTIVE_WINDOW_BOX_COLOR : COLOR_WINDOW_BOX_COLOR));
        box(wnd, 0 , 0);

        size_t offset = title_offset;
        size_t title_len = title.length();
        std::string t = title;

        if (winrect.w()-2<=title_len) {
            t = t.substr(0, winrect.w()-2);
            offset = 1;
        } else 
        if (title_offset==TITLE_OFFSET_LEFT){
            offset=1;
        }
        else 
        if (title_offset==TITLE_OFFSET_RIGHT){
            offset=winrect.w()-1-title_len;
        }
        else 
        if (title_offset==TITLE_OFFSET_MIDDLE){
            offset = winrect.w()/2 - title_len/2;
        } else {
            assert(false); // invalid offset
        }

        wbkgdset(wnd, COLOR_PAIR(active ? COLOR_ACTIVE_WINDOW_TITLE : COLOR_WINDOW_TITLE));
        mvwprintw(wnd, 0, offset, "%s", t.c_str());
    }

    return true;
}

// offset >= 0 - absolute offset or one of the TITLE_OFFSET_RIGHT, TITLE_OFFSET_LEFT, TITLE_OFFSET_MIDDLE
void TUWindow::set_box(bool b, const std::string& t, int offset) {
    boxed=b;
    title = t;
    title_offset = offset;
    update_canvas();
}

void TUWindow::set_color(int cp) {
    wnd_color = cp;
}

void TUWindow::set_active(bool a) {
    active=a;
}

bool TUWindow::is_active() const {
    return active;
}

// if returned true then message was processed
bool TUWindow::handler(wint_t ch, int err) {
    return false;
}

void TUWindow::put_text(int cx, int cy, const std::wstring& ws) {
    int tlen = ws.length();
    const TURect& canv = canvas();
    assert(cx>=0);
    assert(cy>=0);
    assert(tlen+cx <= canv.w());
    assert(cy < canv.h());

    mvwaddwstr(wnd, to_wnd_y(cy), to_wnd_x(cx), ws.c_str());
}

void TUWindow::put_cursor(int cx, int cy) {
    const TURect& canv = canvas();
    assert(cx>=0);
    assert(cy>=0);
    assert(cx < canv.w());
    assert(cy < canv.h());

    // set cursor 
    int x=to_wnd_x(cx);
    int y = to_wnd_y(cy);
    wmove(wnd, y,x);
}

void TUWindow::put_vert_line(int cx, int cy, int len, wchar_t wc, int cp) {
    cchar_t c;
    setcchar(&c, &wc , A_NORMAL, cp, nullptr);
    for (int i=0; i<len; ++i) {
        mvwadd_wch(wnd, to_wnd_y(cy+i),to_wnd_x(cx),&c);
    }
}
void TUWindow::put_horiz_line(int cx, int cy, int len, wchar_t wc, int cp) {
    cchar_t c;
    setcchar(&c, &wc , A_NORMAL, cp, nullptr);
    for (int i=0; i<len; ++i) {
        mvwadd_wch(wnd, to_wnd_y(cy),to_wnd_x(cx+i),&c);
    }    
}

void TUWindow::set_index(int indx) {
    assert(indx>=0);
    index = indx;
}

int TUWindow::get_index() const {
    return index;
}

void TUWindow::destroy() {
    delwin(wnd);        
    wnd = nullptr;
}

const TURect& TUWindow::canvas() const {
    return canvrect;
}

const TURect& TUWindow::winarea() const {
    return winrect;
}

int TUWindow::to_wnd_x(int cx) const {
    return cx+canvrect.x();
}
int TUWindow::to_wnd_y(int cy) const {
    return cy+canvrect.y();
}

// ********************************** TUScrollWindow **********************************
// Objective: Window with scrolling support.
//*****************************************************************************
TUScrollWindow::TUScrollWindow (IntValFn _x, IntValFn _y, IntValFn _w, IntValFn _h) : super(_x, _y, _w, _h){
    set_scroll_bar(SCROLLBAR_OFF,SCROLLBAR_OFF);
}

TUScrollWindow::~TUScrollWindow() {

}

TURect TUScrollWindow::set_scroll_area(int x, int y, bool relative) {
    // update data area
    data_area = data_dim();
    const TURect& canv = canvas();
    int cx,cy,cw,ch;
    canv.get(cx, cy, cw, ch);

    // Calculate scroll are
    if (relative) {
        scroll_area = TURect(scroll_area.x() + x, scroll_area.y() + y, cw, ch);
    } else {
        scroll_area = TURect(x, y, cw, ch);
    }

    correct_scroll_area();

    return scroll_area;
}

void TUScrollWindow::set_scroll_bar(int xbar, int ybar) {
    assert( (xbar==SCROLLBAR_OFF) || (xbar==SCROLLBAR_ON) || (xbar==SCROLLBAR_AUTO) );
    assert( (ybar==SCROLLBAR_OFF) || (ybar==SCROLLBAR_ON) || (ybar==SCROLLBAR_AUTO) );
    scroll_bar_x = xbar;
    scroll_bar_y = ybar;   
    update_canvas();        
}
/*
bool TUScrollWindow::in_data(int x, int y) {
    return data_area.belong(x,y);
}

bool TUScrollWindow::is_empty() const {
    return data_w==0 && data_h==0;
}
*/

void TUScrollWindow::update_canvas() {
    super::update_canvas();
    int x,y,w,h;
    super::canvas().get(x, y, w, h);
    scrollcanv = TURect(0, 0, w, h);

    draw_scroll_bar_x = scroll_bar_x==SCROLLBAR_ON || (scroll_bar_x==SCROLLBAR_AUTO && data_area.w() > scrollcanv.w());
    draw_scroll_bar_y = scroll_bar_y==SCROLLBAR_ON || (scroll_bar_y==SCROLLBAR_AUTO && data_area.h() > scrollcanv.h());

    if (draw_scroll_bar_y) {
        y_scroll_area = TURect(scrollcanv.w() - 1, 0, 1, scrollcanv.h() - draw_scroll_bar_x);
    } else {
        y_scroll_area = TURect();
    }

    if (draw_scroll_bar_x) {
        x_scroll_area = TURect(0, scrollcanv.h() - 1, scrollcanv.w() - draw_scroll_bar_y, 1);
    } else {
        x_scroll_area = TURect();
    }

    scrollcanv.transform(0,0,-draw_scroll_bar_y, -draw_scroll_bar_x);    
}

void TUScrollWindow::correct_scroll_area() {
    int w = std::min(data_area.w(), scrollcanv.w());
    int h = std::min(data_area.h(), scrollcanv.h());

    int x = std::min(scroll_area.x(), data_area.w() - scrollcanv.w());
    x = std::max(x, 0);

    int y = std::min(scroll_area.y(), data_area.h() - scrollcanv.h());
    y = std::max(y, 0);

    scroll_area = TURect(x, y, w, h);
}

bool TUScrollWindow::handler(wint_t ch, int err) {
    if (err!=KEY_CODE_YES) {
        return false;
    } 

    switch(ch) {
    case KEY_LEFT:
        set_scroll_area(-1,0, true);
    break;
    case KEY_RIGHT:
        set_scroll_area(1,0, true);
    break;
    case KEY_UP:
        set_scroll_area(0,-1, true);
    break;
    case KEY_DOWN:
        set_scroll_area(0,1, true);
    break;  
    case KEY_HOME:
        set_scroll_area(0,0, false);
    break;
    case KEY_END:
        set_scroll_area(data_area.w(),data_area.h(), false);
    break;
    case KEY_PPAGE:
        set_scroll_area(0,-scrollcanv.h(), true);
    break;
    case KEY_NPAGE:
        set_scroll_area(0,scrollcanv.h(), true);
    break;

    default:
        return false;
    }

    return true;
}

void TUScrollWindow::draw_scrolls() {
    int x,y,w,h;    // scroll bar rectangle
    int dx,dy,dw,dh;// data area
    int cx,cy,cw,ch;// canvas area
    int sx,sy,sw,sh;// scroll area;
    int from, to;
    wchar_t scr_hor_ch    = 0x2501;
    wchar_t scr_hor_space = L' ';
    wchar_t scr_vert_ch    = 0x2503;
    wchar_t scr_vert_space = L' ';

    scroll_area.get(sx, sy, sw, sh);
    data_area.get(dx, dy, dw, dh);
    scrollcanv.get(cx, cy, cw, ch);

    if (draw_scroll_bar_x) {
        x_scroll_area.get(x, y, w, h);

        if (dw <= cw) {
            from = 0;
            to = w;
        } else {
            from = sx*w/dw;
            to = from + std::max(sw*w/dw,1);
        }

        put_horiz_line(x,y,from,scr_hor_space,COLOR_TEXT);
        put_horiz_line(x+from,y,to-from,scr_hor_ch,COLOR_TEXT);
        put_horiz_line(x+to,y,w-to,scr_hor_space,COLOR_TEXT);
    } 

    if (draw_scroll_bar_y) {
        y_scroll_area.get(x, y, w, h);
        if (dh <= ch) {
            from = 0;
            to = h;
        } else {
            from = sy*h/dh;
            to = from + std::max(sh*h/dh,1);
        }

        put_vert_line(x,y,from,scr_vert_space,COLOR_TEXT);
        put_vert_line(x,y+from,to-from,scr_vert_ch,COLOR_TEXT);
        put_vert_line(x,y+to,h-to,scr_vert_space,COLOR_TEXT);
    } 
}

bool TUScrollWindow::redraw() {
    if (!super::redraw()) {
        return false;
    }

    // update scrolling position before drawing
    set_scroll_area(0,0, true);
    draw_scrolls();      
    redraw_area(scroll_area);
    return true;
}

bool TUScrollWindow::drawable() const {
    int x,y,w,h;
    super::canvas().get(x, y, w, h);
    TURect canv(0, 0, w, h);
    bool res = super::drawable() &&
               (!draw_scroll_bar_x || x_scroll_area.drawable(canv)) &&
               (!draw_scroll_bar_y || y_scroll_area.drawable(canv));
    return res;
}   

const TURect& TUScrollWindow::canvas() const {
    return scrollcanv;
}

int TUScrollWindow::to_wnd_x(int cx) const {
    return super::to_wnd_x(cx+scrollcanv.x());
}
int TUScrollWindow::to_wnd_y(int cy) const {
    return super::to_wnd_y(cy+scrollcanv.y());
}




// ********************************** TUTextWindow **********************************
// Objective: Window with text viewing support.
//*****************************************************************************


TUTextWindow::TUTextWindow(IntValFn _x, IntValFn _y, IntValFn _w, IntValFn _h) : super(_x, _y, _w, _h){
    set_color(COLOR_TEXT);
}

TUTextWindow::~TUTextWindow() {

}

void TUTextWindow::update_window(bool scroll_down) {
    if (scroll_down) {
        set_scroll_area(0, text.size(), false);
    }    
}

void TUTextWindow::set_text(const TUTextLines& t) {
    size_t tlen = t.size();
    text.clear();
    
    // Conversion to wide string (UTF16LE) and max length calculation    
    for (size_t i=0; i<tlen; ++i) {
        std::wstring ws = tools::utf8_to_wstr(t[i]);
        text.push_back(ws);
    }
    update_window(true);
}


void TUTextWindow::clear() {
    text.clear();
    update_window(true);
}

void TUTextWindow::append(const TUTextLines& t) {
    size_t t_lines = t.size();

    // Conversion to wide string (UTF16LE) and max length calculation
    for (size_t i=0; i<t_lines; ++i) {
        std::wstring ws = tools::utf8_to_wstr(t[i]);
        text.push_back(ws);
    }

    update_window(auto_scroll);
}

void TUTextWindow::append(const std::string& l) {
    TUTextLines lines = tools::split_and_trim(l, [](char c){return c == '\n';}, [](char c){return false;});
    append(lines);
}

TURect TUTextWindow::data_dim() const{
    int max_w = 0;

    for (const std::wstring& ws : text) {
        max_w = std::max(max_w, (int)ws.length());
    }
    return TURect(0, 0, max_w, text.size());
}

bool TUTextWindow::redraw_area(const TURect& area) {
    int fl = area.y();
    int ll = area.lbc().y();
    int off_x = area.lbc().x();

    wbkgdset(wnd, COLOR_PAIR(COLOR_TEXT));
    for (int i = fl; i<=ll; ++i) {
        std::wstring ws = text[i];
        size_t len = ws.length();
        if (off_x<len) {
            std::wstring wss = ws.substr(off_x, area.w());
            put_text(0, i-fl, wss);
        }
    }
    return true;
}

bool TUTextWindow::handler(wint_t ch, int err) {
    return super::handler(ch, err);
}

// ********************************** TUTextWindow **********************************
// Objective: Window that allows editing of the single line
//*****************************************************************************

TUInputWindow::TUInputWindow(IntValFn _x, IntValFn _y, IntValFn _w, IntValFn _h) : super(_x, _y, _w, _h){
    set_color(COLOR_TEXT);
}

TUInputWindow::~TUInputWindow(){
}

TURect TUInputWindow::data_dim() const {
    int len = text.length();
    const TURect& canv = canvas();
    int x,y,w,h;
    canv.get(x, y, w, h);

    // one more character for empty cursor place
    assert(cur_pos<=len);
    assert(cur_pos>=0);
    len += (cur_pos==len);

    h = 1 + ((len - 1)/w);
    return TURect(0, 0, w, h);
}

bool TUInputWindow::redraw_area(const TURect& area) {
    int tlen = text.length();
    int x,y,w,h;
    area.get(x, y, w, h);
    assert(x==0);

    min_cur_pos = y*w;
    max_cur_pos = std::min(min_cur_pos + w*h, tlen);

   
    assert(min_cur_pos<=tlen);
    assert(max_cur_pos<=tlen);

    wbkgdset(wnd, COLOR_PAIR(COLOR_TEXT));

    int nlines = std::min(1 + (tlen-min_cur_pos) / w, h);
    for (int i = 0; i<nlines; ++i) {
        std::wstring wss = text.substr(min_cur_pos + i*w, w);
        put_text(0,i,wss);
    }

    if (is_active()) {
        assert(cur_pos<=tlen);
        assert(cur_pos>=0);        

        if (cur_pos<min_cur_pos) {
            cur_pos = min_cur_pos;
        } else
        if (cur_pos==tlen) {
            cur_pos = std::min(tlen, min_cur_pos + w*h - 1);
        } else
        if (cur_pos>=min_cur_pos+w*h) {
            cur_pos = min_cur_pos + w*h - 1;
        }

        put_cursor((cur_pos - min_cur_pos) % w, (cur_pos - min_cur_pos) / w );
    }

    return true;
}

void TUInputWindow::set_text(const std::string& t) {
    text = tools::utf8_to_wstr(t);
    cur_pos = std::min(cur_pos, (int)text.length());
}

std::string TUInputWindow::get_text() {
    return tools::wstr_to_utf8(text);
}

void TUInputWindow::set_insert_mode(bool mode) {
    insert_mode = mode;
    curs_set(is_active()*(1+insert_mode));
}

void TUInputWindow::handle_char(wint_t ch) {
    int tlen = text.length();
    assert(cur_pos>=0);
    assert(cur_pos<=tlen);
    if (ch==0 && tlen) {
        //remove character
        text.erase(cur_pos, 1); 
    } else 
    if (ch!=0) {
        if (!insert_mode) {
           text.erase(cur_pos, 1);  
        }
        text.insert(cur_pos, 1, ch);
        handle_cursor(1, false);
    }
}

void TUInputWindow::handle_cursor(int pos_change, bool absolute) {
    int tlen = text.length();
    int scroll_by_y = 0;

    if (absolute) {
        cur_pos=pos_change;    
    } else {
        cur_pos+=pos_change;    
    }

   
    if (cur_pos<0) {
        cur_pos=0;
    } else 
    if (cur_pos>tlen){
        cur_pos = tlen;
    }

    if (cur_pos<min_cur_pos) {
        // scroll up
        scroll_by_y = -( ((min_cur_pos - cur_pos) / canvas().w()) + 1);

    } else if (cur_pos>=max_cur_pos) {
        // scroll down
        scroll_by_y = ((cur_pos-max_cur_pos) / canvas().w()) + 1;
    }

    if (scroll_by_y) {
        TURect new_area = set_scroll_area(0, scroll_by_y, true);
        min_cur_pos = new_area.y()*new_area.w();
        max_cur_pos = std::min(min_cur_pos+new_area.w()*new_area.h(), tlen);
        assert(cur_pos>=min_cur_pos);
        assert(cur_pos<=max_cur_pos);
        assert(max_cur_pos<=tlen);
    }
}

bool TUInputWindow::handler(wint_t ch, int err){
        
    bool res = false;
    const TURect& canv = canvas();
    int tlen = text.length();

    if (err==KEY_CODE_YES) {
        // functional key
        res = true;
        switch (ch) {
            case KEY_BACKSPACE:
            if (cur_pos) {
                handle_cursor(-1, false);
                handle_char(0);
            }
            break;

            case KEY_DC:
                if (cur_pos<tlen)
                {
                    handle_char(0);
                }
            break;

            case KEY_LEFT:
                handle_cursor(-1, false);
            break;
            case KEY_RIGHT:
                handle_cursor(1, false);
            break;
            case KEY_UP:
                handle_cursor(-canv.w(), false);
            break;
            case KEY_DOWN:
                handle_cursor(canv.w(), false);
            break;  
            case KEY_HOME:
                handle_cursor(0, true);
            break;
            case KEY_END:
                handle_cursor(std::numeric_limits<int>::max(), true);
            break;
            case KEY_PPAGE:
                handle_cursor(-canv.w()*canv.h(), false);
            break;
            case KEY_NPAGE:
                handle_cursor(canv.w()*canv.h(), false);
            break;            

            case KEY_IC:
                set_insert_mode(!insert_mode);
            break;

            case KEY_EIC:
                set_insert_mode(false);
            break;    

            default:
                res = false;
        }
    } else if (err==OK && ch!=L'\t' && ch!='\n') {
        // just a letter
        handle_char(ch);
        res = true;
    }

    // allow parent window to process scrolling and etc.
    res |= super::handler(ch, err);

    return res;
}

void TUInputWindow::set_active(bool a) {
    super::set_active(a);
    set_insert_mode(insert_mode);
}

TUI::TUI() {
    this_shared = std::shared_ptr<TUI>(this);
    setlocale(LC_ALL, "");        
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    start_color();
    use_default_colors();
    mousemask(ALL_MOUSE_EVENTS, nullptr);
}

void TUI::init() {
    init_colors();
    update_screen();
}

void TUI::add_window(size_t index, TUWndPtr wnd) {
    assert(windows.find(index)==windows.end()); // window is being overwritten
    windows[index] = wnd;
    assert(wnd->get_index()==-1); // was not added before
    wnd->set_index(index);
    set_active_window(index);
    wnd->set_ui(this_shared);
}

void TUI::init_colors() {
    int background = -1;

    init_pair(COLOR_WINDOW_BOX_COLOR,           COLOR_YELLOW,     background);
    init_pair(COLOR_ACTIVE_WINDOW_BOX_COLOR,    background,       COLOR_YELLOW);
    init_pair(COLOR_WINDOW_TITLE,               COLOR_GREEN,      background);
    init_pair(COLOR_ACTIVE_WINDOW_TITLE,        COLOR_RED,        COLOR_GREEN);
    init_pair(COLOR_TEXT,                       COLOR_WHITE,      background);
    init_pair(COLOR_SELECTION,                  background,       COLOR_WHITE);
}

TUI::~TUI() {
    endwin();
}

// negative value: no active windows
void TUI::set_active_window(int index) {
    TUWndPtr prev,next;
    if (active_window>=0) {
        // deactivate window
        prev = windows.at(active_window);
        assert(prev->is_active());
        prev->set_active(false);
    }

    if (index>=0) {
        next = windows.at(index);
        assert(!next->is_active());
        next->set_active(true);
    }
    active_window = index;
}

void TUI::redraw(bool all) {
    for (auto wp : windows) {
        if (all || wp.second->is_active()) {
            wp.second->redraw();
            wrefresh(wp.second->wnd);
        }
    }        
}

int TUI::get_next_window(int index) {
    auto i = windows.find(index);
    if (i==windows.end()) {
        throw std::out_of_range("window was not found");
    }

    i++;
    if (i==windows.end())
        i = windows.begin();

    return i->second->get_index();        
}

int TUI::get_prev_window(int index) {
    auto i = windows.find(index);
    if (i==windows.end()) {
        throw std::out_of_range("window was not found");
    }

    if (i==windows.begin())
        i = --windows.end();
    else --i;

    return i->second->get_index();
}

void TUI::message_handler(int index, wint_t ch, int err) {
}

void TUI::runloop() {
    wint_t ch;
    int err;
    MEVENT mevent;
    redraw(true);
    while(true) {
        TUWndPtr wnd;
        bool handled = false;
        bool all = false;

        assert(active_window>=0);
        assert(windows.size()>0);

        wnd = windows.at(active_window);        
        err = wget_wch(wnd->wnd, &ch);

        // Handle base APP first
        if (err==KEY_CODE_YES) {
            switch(ch) {
                case KEY_RESIZE:
                update_screen();
                all = true;
                handled = true;
                break;

                case KEY_BTAB:
                    all = true;
                    handled = true;
                    set_active_window(get_prev_window(active_window));
                break;

                case KEY_MOUSE:
                    if(getmouse(&mevent) == OK)
                    {
                        handled = true;
                        all = mouse_handler(mevent);
                    }
                break;
            }
        } else 
        if (err==OK) {
            switch(ch)
            {
                case L'\t':
                    all = true;
                    handled = true;
                    set_active_window(get_next_window(active_window));
                break;
            }


        }

        if (!handled) {
            handled = wnd->handler(ch, err);
        }

        if (!handled) {
            message_handler(active_window, ch, err); 
        }

        redraw(all);
    }
}

bool TUI::mouse_handler(const MEVENT& mevent) {
    bool res = false;
    // find related window
    for (auto wp : windows) {
        auto rect = wp.second->winarea();
        if (rect.belong(mevent.x, mevent.y)) {
            if (active_window!=wp.first) {
                set_active_window(wp.first);
                res = true;
            }
            break;
        }
    }

    return res;
}

/// \brief Update screen size
void TUI::update_screen() {
    screen.transform([]() { return 0; },
                     []() { return 0; },
                     []() { return COLS + 1; },
                     []() { return LINES + 1; });
}
