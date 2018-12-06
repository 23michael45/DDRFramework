// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: google/protobuf/unittest_import_public_lite.proto

#ifndef PROTOBUF_INCLUDED_google_2fprotobuf_2funittest_5fimport_5fpublic_5flite_2eproto
#define PROTOBUF_INCLUDED_google_2fprotobuf_2funittest_5fimport_5fpublic_5flite_2eproto

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3006001
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/message_lite.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
// @@protoc_insertion_point(includes)
#define PROTOBUF_INTERNAL_EXPORT_protobuf_google_2fprotobuf_2funittest_5fimport_5fpublic_5flite_2eproto 

namespace protobuf_google_2fprotobuf_2funittest_5fimport_5fpublic_5flite_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[1];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
}  // namespace protobuf_google_2fprotobuf_2funittest_5fimport_5fpublic_5flite_2eproto
namespace protobuf_unittest_import {
class PublicImportMessageLite;
class PublicImportMessageLiteDefaultTypeInternal;
extern PublicImportMessageLiteDefaultTypeInternal _PublicImportMessageLite_default_instance_;
}  // namespace protobuf_unittest_import
namespace google {
namespace protobuf {
template<> ::protobuf_unittest_import::PublicImportMessageLite* Arena::CreateMaybeMessage<::protobuf_unittest_import::PublicImportMessageLite>(Arena*);
}  // namespace protobuf
}  // namespace google
namespace protobuf_unittest_import {

// ===================================================================

class PublicImportMessageLite : public ::google::protobuf::MessageLite /* @@protoc_insertion_point(class_definition:protobuf_unittest_import.PublicImportMessageLite) */ {
 public:
  PublicImportMessageLite();
  virtual ~PublicImportMessageLite();

  PublicImportMessageLite(const PublicImportMessageLite& from);

  inline PublicImportMessageLite& operator=(const PublicImportMessageLite& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  PublicImportMessageLite(PublicImportMessageLite&& from) noexcept
    : PublicImportMessageLite() {
    *this = ::std::move(from);
  }

  inline PublicImportMessageLite& operator=(PublicImportMessageLite&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  inline const ::std::string& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }
  inline ::std::string* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const PublicImportMessageLite& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const PublicImportMessageLite* internal_default_instance() {
    return reinterpret_cast<const PublicImportMessageLite*>(
               &_PublicImportMessageLite_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  void Swap(PublicImportMessageLite* other);
  friend void swap(PublicImportMessageLite& a, PublicImportMessageLite& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline PublicImportMessageLite* New() const final {
    return CreateMaybeMessage<PublicImportMessageLite>(NULL);
  }

  PublicImportMessageLite* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<PublicImportMessageLite>(arena);
  }
  void CheckTypeAndMergeFrom(const ::google::protobuf::MessageLite& from)
    final;
  void CopyFrom(const PublicImportMessageLite& from);
  void MergeFrom(const PublicImportMessageLite& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  void DiscardUnknownFields();
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(PublicImportMessageLite* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::std::string GetTypeName() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 e = 1;
  bool has_e() const;
  void clear_e();
  static const int kEFieldNumber = 1;
  ::google::protobuf::int32 e() const;
  void set_e(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:protobuf_unittest_import.PublicImportMessageLite)
 private:
  void set_has_e();
  void clear_has_e();

  ::google::protobuf::internal::InternalMetadataWithArenaLite _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  ::google::protobuf::int32 e_;
  friend struct ::protobuf_google_2fprotobuf_2funittest_5fimport_5fpublic_5flite_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// PublicImportMessageLite

// optional int32 e = 1;
inline bool PublicImportMessageLite::has_e() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void PublicImportMessageLite::set_has_e() {
  _has_bits_[0] |= 0x00000001u;
}
inline void PublicImportMessageLite::clear_has_e() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void PublicImportMessageLite::clear_e() {
  e_ = 0;
  clear_has_e();
}
inline ::google::protobuf::int32 PublicImportMessageLite::e() const {
  // @@protoc_insertion_point(field_get:protobuf_unittest_import.PublicImportMessageLite.e)
  return e_;
}
inline void PublicImportMessageLite::set_e(::google::protobuf::int32 value) {
  set_has_e();
  e_ = value;
  // @@protoc_insertion_point(field_set:protobuf_unittest_import.PublicImportMessageLite.e)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace protobuf_unittest_import

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_INCLUDED_google_2fprotobuf_2funittest_5fimport_5fpublic_5flite_2eproto
