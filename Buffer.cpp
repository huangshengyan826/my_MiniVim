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

    std::ifstream file(path_.string());

    if (file.is_open()){
        std::string line;
        while (std::getline(file,line)){
            lines_.push_back(line);
        }
        
        if (lines_.empty()){
            lines_.push_back("");
        }
        
        file.close();
    }
    //从path指向的文件构造Buffer,你需要打开文件并且把文件内容填充进Buffer,并正确初始化一些状态.
    //注意path可能为空的边界情况
}

Buffer::Buffer(std::vector<std::string> lines, std::filesystem::path path) {
    lines_ = lines;
    path_ = path;
    if (lines_.empty()) {
        lines_.push_back("");
    }
}

std::size_t Buffer::GetLineCount() const {
    //返回文件行数
    return lines_.size();
}

const std::string& Buffer::GetLineAt(std::size_t row) const {
    //返回第row行的内容
    if (lines_.empty()) {
        return "";
    }
    if (row >= lines_.size()) {
        return lines_.back(); 
    }
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
    lines_[row].insert(column,1,value);
    is_modified_ = true;
   }
}

void Buffer::EraseCharacter(std::size_t row, std::size_t column) {
   //在第row行第col列删除一个value
   if (row<lines_.size() && column<lines_[row].length()){
    lines_[row].erase(column,1);
    is_modified_ = true;
   }
}

void Buffer::SplitLine(std::size_t row, std::size_t column) {
    //在第row行第col列分割,即在此处敲了回车键
    if (row<lines_.size() && column<=lines_[row].length()){
        std::string right_part = lines_[row].substr(column);
        std::string left_part = lines_[row].substr(0,column);
        lines_[row] = left_part;
        lines_.insert(lines_.begin()+row+1,right_part);
        
        is_modified_ = true;
    }
}

void Buffer::JoinLine(std::size_t row) {
   //把第row + 1行合并进第row行
   if (row + 1 < lines_.size()){
    lines_[row] += lines_[row+1];
    lines_.erase(lines_.begin()+row+1);

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
    path_ = path;
    WriteTo(path_);
    
    is_modified_ = false;
}


void Buffer::WriteTo(const std::filesystem::path& path) const {
   //实际将缓冲区中的内容写入path指向的文件中
   std::ofstream file(path.string());

   if (file.is_open()){
    for (std::size_t i=0;i<lines_.size();++i){
        file << lines_[i] << "\n";
    }
    file.close();
   }
}

} // namespace sjtu
