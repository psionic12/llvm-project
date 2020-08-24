#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_RECORD_INFO_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_RECORD_INFO_H_
enum EntryKind {
  TypeBool,
  TypeChar,
  TypeUchar,
  TypeFloat,
  TypeDouble,
  TypeInt16,
  TypeUint16,
  TypeInt32,
  TypeUint32,
  TypeInt64,
  TypeUint64,
  TypePointer,
};
class EntryInfo {
public:
public:
  void AddCategory(clang::StringRef category) {
    categories_.emplace_back(category);
  }
  void SetType(std::string &&type) { type_ = std::move(type); }
  void SetName(std::string &&name) { name_ = std::move(name); }
  void SetRepeated(bool repeated) { repeated_ = repeated; }
  void SetEntryKind(EntryKind kind) { kind_ = kind; }

private:
  std::string type_;
  std::string name_;
  std::vector<clang::StringRef> categories_;
  bool repeated_ = false;
  EntryKind kind_;
};

class RecordInfo {
public:
  void AddBase(const RecordInfo *base) { bases_.push_back(base); }
  void SetToInvalid() { invalid_ = true; }
  bool Invalid() { return invalid_; }

private:
  bool invalid_ = false;
  std::vector<const RecordInfo *> bases_;
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_RECORD_INFO_H_
