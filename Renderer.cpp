#include "Renderer.hpp"
#include "TextLayout.hpp"

namespace sjtu {

//我们在Render中保留了所有需要用到控制序列的部分逻辑,你不应该修改它们.
//啥是控制序列?它们是嵌入在文本流中的“指令”,告诉终端执行特定操作(比如移动光标、改变文字颜色、清屏或清除行内容).   
namespace {

void AppendClearedLine(std::string& frame, std::string_view contents, std::size_t width, bool newline) {
    //追加不超过width列的显示内容,再用ESC[K清除这一行余下的旧内容
    //需要换行时同时输出回车和换行,因为raw模式关闭了终端的自动输出转换
    //你在生成帧的时候生成每一行都应该调用这个函数
    frame += "\x1b[2K";
    frame += "\r";
    frame.append(contents.substr(0, width));
    if (newline) {
        frame += "\r\n";
    }
}

std::string CursorSequence(std::size_t row, std::size_t column) {
    //生成定位光标的ANSI转义序列,这里接收的行列都从1开始
    return "\x1b[" + std::to_string(row) + ';' + std::to_string(column) + 'H';
}

}

std::string Renderer::Render(const Buffer& buffer, const Window& window, const RenderState& state) const {
    //1. 隐藏光标并回到屏幕左上角,开始拼接新的一帧(一帧就是一个长string)
    //2. 按视口逐行取Buffer内容,展开Tab后再横向截取;超出文件的行显示~
    //3. 绘制底部一行,优先显示命令行,其次是提示信息,最后是Insert模式标记
    //4. 根据当前模式计算光标的屏幕位置,追加定位序列并重新显示光标
    //你应该区分我们的Cursor光标(它代表当前用户在修改文件的哪个位置)和终端显示的白色亮条,白色亮条在屏幕中的位置是由光标位置和视口位置计算的
    auto& viewport = window.GetViewport();
    auto width = std::max<std::size_t>(viewport.columns_, 1);

    std::string frame;
    frame.reserve((viewport.rows_ + 1) * (width + 8));
    frame += "\x1b[?25l";
    frame += "\x1b[H";

    for (std::size_t screen_row = 0; screen_row < viewport.rows_; ++screen_row) {
        std::size_t buffer_row = screen_row + viewport.top_;

        bool need_newline = (screen_row + 1 < viewport.rows_);

        if (buffer_row>=buffer.GetLineCount()){
            AppendClearedLine(frame,"~",width,need_newline);
        }
        else{
            std::string expanded = ExpandForDisplay(buffer.GetLineAt(buffer_row));

            std::string visible_part="";
            if (expanded.length() > viewport.left_){
                visible_part = expanded.substr(viewport.left_);
            }

            AppendClearedLine(frame,visible_part,width,need_newline);
        }
    }

    std::string bottom;
    if (state.mode_ == Mode::CommandLine) {
        bottom = ":" + state.command_;
    }
    else if (!state.message_.empty()) {
        bottom = state.message_;
    }
    else if (state.mode_ == Mode::Insert) {
        bottom = "-- INSERT --";
    }
    AppendClearedLine(frame, bottom, width, false);
    //这里是提示3中的部分
    //我们只会在CommandMode的时候检查一下底部的命令内容,在NormalMode不会看底部,所以message你可以随意写

    std::size_t cursor_row{0};
    std::size_t cursor_column{0};

    if (state.mode_==Mode::CommandLine){
        cursor_row = viewport.rows_ + 1;
        cursor_column = state.command_.length() + 1;
    }
    else{
        Position cursor = window.GetCursor();
        cursor_row = cursor.row_ - viewport.top_ + 1;
        

        std::size_t safe_row = 0;
        if (buffer.GetLineCount() > 0) {
            safe_row = std::min(cursor.row_, buffer.GetLineCount() - 1);
        }
        
        const std::string& line = buffer.GetLineAt(safe_row);
        std::size_t screen_column = BufferColumnToRenderColumn(line, cursor.column_);

        cursor_column = screen_column - viewport.left_ + 1;
    }

    frame += CursorSequence(cursor_row, cursor_column);
    frame += "\x1b[?25h";
    return frame;
}

std::string Renderer::ExpandForDisplay(std::string_view line) {
    std::string expanded;
    std::size_t current_column = 0;

    for (char c : line){
        if (c == '\t'){
            std::size_t spaces = tabStop - (current_column % tabStop);
            expanded.append(spaces,' ');
            current_column += spaces;
        }
        else{
            expanded.push_back(c);
            current_column++;
        }
    }
    return expanded;
    //从左到右扫描buffer中一整行的实际内容,并扩展到render应该输出的视图
    //你应该在Render中调用这个函数,并把函数返回的结果按照视口剪切用于Render的某些行
    
}
} // namespace sjtu
