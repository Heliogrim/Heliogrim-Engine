#pragma once
#include <set>

#include <Engine.Common/Collection/List.hpp>
#include "CuStagedBuffer.hpp"
#include "Device.hpp"
#include "Texture/TextureArray.hpp"
#include "Uniform/UniformIndex.hpp"
#include "Uniform/UniformInstance.hpp"
#include "Uniform/UniformMaterial.hpp"
#include "Uniform/UniformVertex.hpp"

namespace clockwork::__internal::gfx::types {
    template <typename Type>
    class CacheContainer {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 25.06.2020
         *
         * @param  ptr_ The pointer.
         * @param  count_ Number of.
         */
        CacheContainer(const Type*& ptr_, const size_t count_) :
            _count(count_),
            _ptr(ptr_) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 25.06.2020
         *
         * @param [in,out] ptr_ If non-null, the pointer.
         * @param 		   count_ Number of.
         */
        CacheContainer(Type* ptr_, const size_t count_) :
            _count(count_),
            _ptr(ptr_) {}

    private:
        /**
         * Count
         */
        size_t _count;

    public:
        /**
         * Gets the count
         *
         * @author Julius
         * @date 25.06.2020
         *
         * @returns A size_t.
         */
		FORCEINLINE size_t count() const {
            return _count;
        }

        /**
         * Gets the size
         *
         * @author Julius
         * @date 25.06.2020
         *
         * @returns A size_t.
         */
		FORCEINLINE size_t size() const {
            return _count * sizeof(Type);
        }

    private:
        /**
         * Ptr
         */
        const Type* _ptr;

    public:
        /**
         * Gets the begin
         *
         * @author Julius
         * @date 25.06.2020
         *
         * @returns Null if it fails, else a pointer to a const Type.
         */
        const Type* begin() const {
            return _ptr;
        }

        /**
         * Gets the begin
         *
         * @author Julius
         * @date 25.06.2020
         *
         * @returns Null if it fails, else a pointer to a const Type.
         */
        const Type* begin() {
            return _ptr;
        }
    };

    typedef CacheContainer<UniformIndex> IndexContainer;
    typedef CacheContainer<UniformInstance> InstanceContainer;
    typedef CacheContainer<UniformMaterial> MaterialContainer;
    typedef CacheContainer<UniformVertex> VertexContainer;

    namespace {
        template <class Type, vk::BufferUsageFlagBits UsageFlags, class BufferType = CuStagedBuffer>
        class Cache {
        public:
            using internal_buffer_type = clockwork::__internal::gfx::types::Buffer;
            using container_type = CacheContainer<Type>;
            using reference_type = __CacheReference<Type>;

            static_assert(_STD is_base_of<clockwork::__internal::gfx::types::CacheUsable, BufferType>::value);

            /**
             * Default constructor
             *
             * @author Julius
             * @date 25.06.2020
             */
            template <class PseudoBufferType = BufferType, typename = _STD enable_if_t<_STD is_default_constructible_v<
                PseudoBufferType> && _STD is_same_v<PseudoBufferType, BufferType>>>
            Cache(const __Device& device_) :
                _device(device_),
                _buffer(new BufferType()),
                _capacity(0),
                _used(0) {}

            /**
             * Constructor
             *
             * @author Julius
             * @date 25.06.2020
             *
             * @param  device_ The device.
             * @param  buffer_ The buffer.
             */
            Cache(const __Device& device_, BufferType* buffer_) :
                _device(device_),
                _buffer(buffer_),
                _capacity(_buffer->capacity() / sizeof(Type)),
                _used(0) {}

            /**
             * Destructor
             *
             * @author Julius
             * @date 25.06.2020
             */
            ~Cache() {
                _buffer->destroy();
                _register.clear();
            }

        private:
            /**
             * Device
             */
            __Device _device;

        public:
            const __Device& device() const {
                return _device;
            }

        private:
            /**
             * Buffer
             */
            BufferType* _buffer;

        public:
            /**
             * Gets the buffer
             *
             * @author Julius
             * @date 25.06.2020
             *
             * @returns A reference to a const BufferType.
             */
            const BufferType& buffer() const {
                return *_buffer;
            }

        private:
            /**
             * Register
             */
            _STD set<reference_type, _STD less<reference_type>> _register;

        private:
            /**
             * Meta Count
             */
            size_t _capacity;
            size_t _used;

        public:
            /**
             * Gets the capacity
             *
             * @author Julius
             * @date 25.06.2020
             *
             * @returns A size_t.
             */
            size_t capacity() {
                return _capacity;
            }

            /**
             * Gets the capacity
             *
             * @author Julius
             * @date 25.06.2020
             *
             * @returns A reference to a const size_t.
             */
            const size_t& capacity() const {
                return _capacity;
            }

            /**
             * Gets the used
             *
             * @author Julius
             * @date 25.06.2020
             *
             * @returns A size_t.
             */
            size_t used() {
                return _used;
            }

            /**
             * Gets the used
             *
             * @author Julius
             * @date 25.06.2020
             *
             * @returns A reference to a const size_t.
             */
            const size_t& used() const {
                return _used;
            }

        private:
            /**
             * Mutations
             */

        public:
            /**
             * Reserves up to the given new capacity
             *
             * @author Julius
             * @date 25.06.2020
             *
             * @param  newCapacity_ The new capacity.
             */
            void reserve(const size_t newCapacity_) {
                /**
                 * Check whether current capacity is higher
                 */
                if (_capacity >= newCapacity_) {
                    return;
                }

                resize(newCapacity_);
            }

            /**
             * Stores the given payload
             *
             * @author Julius
             * @date 25.06.2020
             *
             * @param  payload_ The payload to store.
             *
             * @returns A reference_type.
             */
            reference_type store(const container_type& payload_) {
                /**
                 * Check whether current buffer has the chance to container a segment to store payload
                 */
                if (_capacity - _used < payload_.count()) {
                    resize(_capacity + payload_.count());
                }

                /**
                 * Search for a segment to contain the payload
                 */
                size_t offset = 0;
                if (!findSegment(payload_.size(), offset)) {
                    /**
                     * If no segment is found, resize the buffer and repeat action
                     */
                    resize(_capacity + payload_.count());
                    return store(payload_);
                }

                /**
                 * Make a reference object from found data
                 */
                reference_type ref = shared<reference_type::element_type>(
                    payload_.size(),
                    offset
                );

                /**
                 * Create a temporary buffer to hold payload
                 */
                Buffer b = _device->createBuffer(
                    vk::BufferUsageFlagBits::eTransferSrc,
                    vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
                    ref->size(),
                    payload_.begin()
                );

                b.bind();

                /**
                 * Patch the segment, store reference to segment list and update stats
                 */
                patch(ref, b);
                _register.insert(ref);
                _used += ref->count();

                /**
                 * Destroy temporary buffer
                 */
                b.destroy();

                /**
                 * Return reference object
                 */
                return ref;
            }

            /**
             * Stores the given payload
             *
             * @author Julius
             * @date 25.06.2020
             *
             * @param  payload_ The payload to store.
             *
             * @returns A reference_type.
             */
            reference_type store(const container_type&& payload_) {
                return store(payload_);
            }

            /**
             * Shrink to fit
             *
             * @author Julius
             * @date 25.06.2020
             */
            [[deprecated]] void shrinkToFit() {
                /**
                 * Check whether there is any space left
                 */
                if (_capacity <= _used) {
                    return;
                }

                _buffer->requestShrink(_used);
            }

            /**
             * Updates segment by reference with new data
             *	if necessary, update reference meta data to new segment
             *
             * @author Julius
             * @date 25.06.2020
             *
             * @param [in,out] reference_ The reference.
             * @param 		   payload_ The payload.
             */
            void update(reference_type& reference_, const container_type& payload_) {
                /**
                 * Check whether segment can contain payload
                 */
                if (reference_->count() < payload_.count()) {
                    return;
                }

                const size_t newSize = payload_.size();
                // reference_->size() = newSize;

                /**
                 * Create temporary buffer to hold payload
                 */
                Buffer b = _device->createBuffer(
                    vk::BufferUsageFlagBits::eTransferSrc,
                    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCached,
                    newSize,
                    payload_.begin()
                );

                b.bind();

                /**
                 * Patch the segment with new updated data
                 */
                patch(reference_, b);

                /**
                 * Destroy temporary buffer
                 */
                b.destroy();
            }

            /**
             * Updates segment by reference with new data
             *	If necessary, update reference meta data to new segment
             *
             * @author Julius
             * @date 25.06.2020
             *
             * @param [in,out] reference_ The reference.
             * @param 		   payload_ The payload.
             */
            void update(reference_type& reference_, const container_type&& payload_) {
                return update(reference_, payload_);
            }

        private:
            /**
             * Searches for available segment
             *
             * @author Julius
             * @date 25.06.2020
             *
             * @param 		   size_ The size requested by size_ = count * sizeof(Type).
             * @param [in,out] offset_ The offset out reference to return offset in buffer.
             *
             * @returns True if it succeeds, false if it fails.
             */
            bool findSegment(const size_t& size_, OUT size_t& offset_) {

                /**
                 * Iterate over all segments to find matching space
                 */
                size_t lastEnd = 0;
                for (auto& entry : _register) {
                    /**
                     * Check whether the distance from last segment to current is enough to fit size_
                     */
                    if ((entry->offset() - lastEnd) >= size_) {
                        offset_ = lastEnd;
                        return true;
                    }

                    /**
                     * If not, move forward
                     */
                    lastEnd = entry->offset() + entry->size();
                }

                /**
                 * If we have the last segment ending,
                 *	check whether size_ fits into space between last segment and buffer capacity
                 */
                if ((_capacity * sizeof(Type)) >= lastEnd + size_) {
                    offset_ = lastEnd;
                    return true;
                }

                /**
                 * No suitable segment found
                 */
                return false;
            }

            /**
             * Patches a segment with given buffer
             *
             * @author Julius
             * @date 25.06.2020
             *
             * @param  segment_ The segment.
             * @param  buffer_ The buffer.
             */
            void patch(const reference_type& segment_, const internal_buffer_type& buffer_) {
                const vk::BufferCopy cpy = {
                    0,
                    segment_->offset(),
                    segment_->size()
                };

                return _buffer->copyTo(cpy, buffer_);
            }

            /**
             * Resizes the internal buffer to given element count
             *
             * @author Julius
             * @date 25.06.2020
             *
             * @param  count_ Number of elements to resize to.
             */
            void resize(const size_t count_) {
                /**
                 * Check whether resize request succeeds
                 */
                if (_buffer->requestResize(count_ * sizeof(Type))) {
                    _capacity = count_;
                }
            }
        };

        typedef Cache<UniformIndex, vk::BufferUsageFlagBits::eIndexBuffer> IndexCache;
        typedef Cache<UniformInstance, vk::BufferUsageFlagBits::eVertexBuffer> InstanceCache;
        typedef Cache<UniformMaterial, vk::BufferUsageFlagBits::eStorageBuffer> MaterialCache;
        typedef Cache<UniformVertex, vk::BufferUsageFlagBits::eVertexBuffer> VertexCache;
    }

    class RenderCache {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 19.06.2020
         *
         * @param  device_ The device.
         */
        RenderCache(const __Device& device_) :
            _device(device_),
            _index(nullptr),
            _instance(nullptr),
            _material(nullptr),
            _vertex(nullptr) { }

        /**
         * Destructor
         *
         * @author Julius
         * @date 19.06.2020
         */
        ~RenderCache() {
            delete _index;
            delete _instance;
            delete _material;
            delete _vertex;
        };

    private:
        /**
         * Device
         */
        __Device _device;

    private:
        /**
         * Texture Array
         */
        __ptr__TextureArray _albedo;
        __ptr__TextureArray _diffuse;
        __ptr__TextureArray _displacement;
        __ptr__TextureArray _metalness;
        __ptr__TextureArray _normal;
        __ptr__TextureArray _roughness;
        __ptr__TextureArray _specular;

    public:
        const TextureArray& albedo() const {
            return *_albedo;
        }

        const TextureArray& diffuse() const {
            return *_diffuse;
        }

        const TextureArray& displacement() const {
            return *_displacement;
        }

        const TextureArray& metalness() const {
            return *_metalness;
        }

        const TextureArray& normal() const {
            return *_normal;
        }

        const TextureArray& roughness() const {
            return *_roughness;
        }

        const TextureArray& specular() const {
            return *_specular;
        }

    private:
        /**
         * Index Cache
         */
        Cache<UniformIndex, vk::BufferUsageFlagBits::eIndexBuffer>* _index;

    public:
        const IndexCache& index() const {
            return *_index;
        }

        IndexCache& index() {
            return *_index;
        }

        const CuStagedBuffer& indexBuffer() const {
            return _index->buffer();
        }

    private:
        /**
         * Instance Cache
         */
        Cache<UniformInstance, vk::BufferUsageFlagBits::eVertexBuffer>* _instance;

    public:
        const InstanceCache& instance() const {
            return *_instance;
        }

        InstanceCache& instance() {
            return *_instance;
        }

        const CuStagedBuffer& instanceBuffer() const {
            return _instance->buffer();
        }

    private:
        /**
         * Material Cache
         */
        Cache<UniformMaterial, vk::BufferUsageFlagBits::eStorageBuffer>* _material;

    public:
        const MaterialCache& material() const {
            return *_material;
        }

        MaterialCache& material() {
            return *_material;
        }

        const CuStagedBuffer& materialBuffer() const {
            return _material->buffer();
        }

    private:
        /**
         * Vertex Cache
         */
        Cache<UniformVertex, vk::BufferUsageFlagBits::eVertexBuffer>* _vertex;

    public:
        const VertexCache& vertex() const {
            return *_vertex;
        }

        VertexCache& vertex() {
            return *_vertex;
        }

        const CuStagedBuffer& vertexBuffer() const {
            return _vertex->buffer();
        }

    private:
        /**
         * Render Model Cache
         */
        clockwork::__internal::types::vector<graphics::model::RenderedModel> _objects;

    public:
        const clockwork::__internal::types::vector<graphics::model::RenderedModel>& objects() const {
            return _objects;
        }

    private:
        /**
         * Mutation
         */

    public:
        /**
         * Atomic Lock of Buffers
         *
         * @author Julius
         * @date 19.06.2020
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool lock() const {
            _index->buffer().barrier().acquire();
            _instance->buffer().barrier().acquire();
            _material->buffer().barrier().acquire();
            _vertex->buffer().barrier().acquire();
            return true;
        }

        /**
         * Atomic Release of Buffers
         *
         * @author Julius
         * @date 19.06.2020
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool release() const {
            _index->buffer().barrier().release();
            _instance->buffer().barrier().release();
            _material->buffer().barrier().release();
            _vertex->buffer().barrier().release();
            return true;
        }

        /**
         * Setups this
         *
         * @author Julius
         * @date 19.06.2020
         */
        void setup() {
            /**
             * Texture Arrays
             */
            _albedo = new TextureArray();
            _diffuse = new TextureArray();
            _displacement = new TextureArray();
            _metalness = new TextureArray();
            _normal = new TextureArray();
            _roughness = new TextureArray();
            _specular = new TextureArray();

            /**
             * Caches
             */
            _index = new IndexCache(
                _device,
                CuStagedBuffer::make(_device, vk::BufferUsageFlagBits::eIndexBuffer, sizeof(UniformIndex))
            );
            _instance = new InstanceCache(
                _device,
                CuStagedBuffer::make(_device, vk::BufferUsageFlagBits::eVertexBuffer, sizeof(UniformInstance))
            );
            _material = new MaterialCache(
                _device,
                CuStagedBuffer::make(_device, vk::BufferUsageFlagBits::eStorageBuffer, sizeof(UniformMaterial))
            );
            _vertex = new VertexCache(
                _device,
                CuStagedBuffer::make(_device, vk::BufferUsageFlagBits::eVertexBuffer, sizeof(UniformVertex))
            );
        }
    };
}
