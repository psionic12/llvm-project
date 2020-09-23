#include "CodeGenerator.h"
#include "CodeGuard.h"
#include <fmt/format.h>
#include <fstream>
SingleHeaderGenerator::SingleHeaderGenerator(
    llvm::StringRef InFile,
    std::unordered_map<const clang::CXXRecordDecl *, RecordInfo> &Cache)
    : InFile(InFile), Cache(Cache) {
  llvm::SmallString<128> Path(InFile);
  llvm::sys::path::replace_extension(Path, ".s11n.h");
  FileName = Path.str().str();
}
SingleHeaderGenerator::~SingleHeaderGenerator() {
  // now generator codes for all records
  std::fstream Generated(FileName, std::ios::out | std::ios::trunc);
  HeaderGuardCoder HeaderGuard(Generated, FileName);
  IncludeCoder IncludeCoder(Generated, InFile);
  for (auto &Pair : Cache) {
    recordGen(Generated, Pair.second);
  }
  Generated.close();
  // update index file
  Database.save();
}
void SingleHeaderGenerator::recordGen(std::fstream &Out,
                                      RecordInfo &RecordInfo) {
  if (!RecordInfo.isSerializable())
    return;
  if (RecordInfo.pure())
    return;

  llvm::StringRef FullName = RecordInfo.fullName();
  auto& Entries = RecordInfo.entries();
  auto &Class = Classes[FullName.str()];

  Out << fmt::format("template <> struct Coder<{}> {{\n", FullName.data());
  Out << fmt::format(
      "static uint8_t *Write(const {} &value, uint8_t *ptr) {{\n",
      FullName.data());
  Out << fmt::format("ptr = WriteRaw(Size(value), ptr);\n");
  for (auto &Entry : Entries) {
    auto EntryID = Class[Entry.entryName().str()];
    Out << fmt::format("ptr = WriteField({}}, value.{}}, ptr);\n", EntryID,
                       Entry.entryName().data());
  }
  Out << fmt::format("return ptr;\n");
  Out << fmt::format("}}\n");

  Out << fmt::format(
      "static const uint8_t *Read({}} &out, const uint8_t *ptr) {{\n",
      FullName.data());
  Out << "    std::size_t total_size;\n"
         "    ptr = ReadRaw(total_size, ptr);\n"
         "    const uint8_t *ptr_end = ptr + total_size;\n"
         "    while (ptr < ptr_end) {\n"
         "      uint32_t index;\n"
         "      ptr = ReadRaw(index, ptr);\n"
         "      uint8_t tag;\n"
         "      ptr = ReadRaw(tag, ptr);\n"
         "      switch (index) {\n";
  for (auto &Entry : Entries) {
    auto EntryID = Class[Entry.entryName().str()];
    Out << fmt::format("case {}:\n", EntryID);
    Out << fmt::format("ptr = ReadRaw(out.f2, ptr);\n", Entry.entryName().data());
    Out << "break;\n";
  }
  Out << "default: {\n"
         "        ptr = SkipUnknown(tag, ptr);\n"
         "      }\n"
         "      }\n"
         "    }\n"
         "    return ptr;\n"
         "  }\n";
  Out << fmt::format("static std::size_t Size(const {} &value) {{\n",
                     FullName.data());
  Out << "std::size_t size = 0;\n";
  for (auto &Entry : Entries) {
    auto EntryID = Class[Entry.entryName().str()];
    Out << fmt::format("size += FieldSize<{}>(value.{});\n", EntryID,
                       Entry.entryName().data());
  }
  Out << "    size += SizeRaw(size);\n"
         "    return size;\n"
         "  }\n";
  Out << "};\n";
}
CoreCppGenerator::CoreCppGenerator() {

}
CoreCppGenerator::~CoreCppGenerator() {

}
void CoreCppGenerator::recordGen(std::fstream &Out) {
  Out << "#include \"base_derive.s11n.h\"\n";
  for (auto Include : Includes) {
    Out << fmt::format("#include \"{}\"\n", Include);
  }
  NamespaceCoder NamespaceMe(Out, "me");
  NamespaceCoder NamespaceS11n(Out, "s11n");
}
void CoreCppGenerator::registerClassID(llvm::StringRef Name, std::size_t ID) {

}
