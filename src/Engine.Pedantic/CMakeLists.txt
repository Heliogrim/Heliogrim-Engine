
# Setup
make_library_sub_project(
        LIB_NAME Engine.Pedantic
        SUB_OF Engine.Common
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)
