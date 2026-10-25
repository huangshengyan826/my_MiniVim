#include <fstream>

#include "Buffer.hpp"

namespace sjtu {

Buffer::Buffer(const std::filesystem::path& path){
    path_ = path;

    if (path_.empty()){
        lines_.push_back("");
        is_modified_ = false;
        return;
    }

    std::ifstream file(path);
    if(!file){
        throw std::runtime_error("Failed to open: " + path.string());
    }
    std::string line;

    while(std::getline(file, line)){
        lines_.push_back(std::move(line));
        line.clear();
    }
    if(file.bad()){
        throw std::runtime_error("Failed to read:" + path.string());
    }
    if(lines_.empty()){
        lines_.push_back("");
    }
    //从path指向的文件构造Buffer,你需要打开文件并且把文件内容填充进Buffer,并正确初始化一些状态.
    //注意path可能为空的边界情况
}

Buffer::Buffer(std::vector<std::string> lines, std::filesystem::path path) {
    lines_ = lines;
    path_ = path;
    if (lines_.empty()) {
        lines_.push_back("");
        is_modified_ = false;
    }
}

std::size_t Buffer::GetLineCount() const {
    //返回文件行数
    return lines_.size();
}

const std::string& Buffer::GetLineAt(std::size_t row) const {
    //返回第row行的内容
    return lines_[row];
}


std::string Buffer::GetDisplayName() const {
    //返回文件名,若是新文件,返回"[No Name]"
    if (path_.empty()){
        return "[No Name]";
    }
    return path_.filename().string();
}

bool Buffer::IsModified() const {
    //返回文件和上次保存比起来是否被修改过
    return is_modified_;
}

void Buffer::InsertCharacter(std::size_t row, std::size_t column, char value) {
    //在第row行第col列插入一个value, 注意越界检查
   if (row<lines_.size() && column<=lines_[row].length()){
        lines_[row].insert(lines_[row].begin() + static_cast<std::ptrdiff_t>(column), value);
        is_modified_ = true;
   }
}

void Buffer::EraseCharacter(std::size_t row, std::size_t column) {
   //在第row行第col列删除一个value
   if (row<lines_.size() && column<=lines_[row].length()){
        lines_[row].erase(lines_[row].begin() + static_cast<ptrdiff_t>(column));
        is_modified_ = true;
   }
}

void Buffer::SplitLine(std::size_t row, std::size_t column) {
    //在第row行第col列分割,即在此处敲了回车键
    if (row<lines_.size() && column<=lines_[row].length()){
        std::string& line = lines_[row];
        std::string tail = line.substr(column);
        line.erase(column);
        lines_.insert(lines_.begin() + static_cast<std::ptrdiff_t>(row) + 1, std::move(tail));
        
        is_modified_ = true;
    }
}

void Buffer::JoinLine(std::size_t row) {
   //把第row + 1行合并进第row行
   if (row + 1 < lines_.size()){
        std::string line = lines_[row + 1];
        lines_.erase(lines_.begin() + static_cast<ptrdiff_t>(row) + 1);
        lines_[row].append(line);

        is_modified_ = true;
   }
}

void Buffer::Save() {
   //把文件内容保存, 直接调用WriteTo方法
   if (path_.empty()){
    return;
   }
   WriteTo(path_);

   is_modified_ = false;
}

void Buffer::SaveAs(const std::filesystem::path& path) {
    if(path.empty()) {
        return;
    }
    path_ = path;
    WriteTo(path_);
    
    is_modified_ = false;
}


void Buffer::WriteTo(const std::filesystem::path& path) const {
   //实际将缓冲区中的内容写入path指向的文件中
   std::ofstream file(path);

   if(!file) {
        throw std::runtime_error("Failed to open: " + path.string());
    }
    if(lines_.size() == 1 && lines_[0].empty()) {
        return;
    }
    for(const auto &line : lines_) {
        file << line << "\n";
    }
}

} // namespace sjtu
