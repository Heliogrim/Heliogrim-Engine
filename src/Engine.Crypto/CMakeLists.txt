
# Setup
make_library_project(
		LIB_NAME Engine.Crypto
		LIB_FOLDER "${IDE_FOLDER}/Engine/Crypto"
		LIBRARIES ${PROJECT_NAME}::Engine.Common
)