#pragma once

#include "../Inc.hpp"
#include <Engine.Resource/Library/SerializationReader.hpp>
#include <Engine.Resource/Library/SerializationWriter.hpp>
#include <Engine.Resource/Library/Serializer.hpp>
#include <Engine.Resource/Library/Url.hpp>

using SerializationReader = clockwork::__internal::resource::types::SerializationReader;
using SerializationWriter = clockwork::__internal::resource::types::SerializationWriter;
using Url = clockwork::__internal::resource::types::Url;

template <typename Ty>
using Serializer = clockwork::__internal::resource::types::Serializer<Ty>;
