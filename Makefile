
NAME = humangl
CXX = g++
CXXFLAGS = -Wall -Werror -Wextra -MMD -MP

UNAME_S := $(shell uname -s)

GLAD_INCLUDE = -Iinclude

PKG_CONFIG ?= pkg-config
GLFW_INCLUDE = $(shell $(PKG_CONFIG) --cflags glfw3 2>/dev/null)
GLFW_LIB = $(shell $(PKG_CONFIG) --libs glfw3 2>/dev/null)

ifeq ($(UNAME_S),Darwin)
	CXXFLAGS += -std=c++20
	CXXFLAGS += -DGL_SILENCE_DEPRECATION

else
	CXXFLAGS += -std=c++2a
endif

GREEN = \033[0;32m
RESET = \033[0m

ifeq ($(DEBUG), 1)
	CXXFLAGS += -g -fsanitize=address
endif

SRC =	src/main.cpp\
		glad.cpp\
		src/modelLoader/RenderModelLoader.cpp\
		src/window/Window.cpp\
		src/inputHandler/InputHandler.cpp\
		src/shaders/Shader.cpp\
		src/shaders/ShaderProgram.cpp\
		src/render/Render.cpp\
		src/render/RenderDraw.cpp\
		src/render/UnitCube.cpp\
		src/texture/Texture.cpp\
		src/texture/BMPLoader.cpp\
		src/matrixMath/MatrixTransform.cpp\
		src/matrixMath/MatrixStack.cpp\
		src/scene/Transformation.cpp\
		src/scene/Arm.cpp\
		src/scene/Humanoid.cpp\
		src/scene/Camera.cpp

OBJ_DIR = obj

OBJ = $(SRC:%.cpp=$(OBJ_DIR)/%.o)
TEST_NAMES = $(OBJ_DIR)/tests/testMatrixTransform $(OBJ_DIR)/tests/testMatrixStack \
	$(OBJ_DIR)/tests/testUnitCube $(OBJ_DIR)/tests/testArm $(OBJ_DIR)/tests/testHumanoid
TEST_RENDER = $(OBJ_DIR)/tests/testRender
TEST_OBJ = $(TEST_NAMES:%=%.o) $(TEST_RENDER).o
TEST_SUPPORT = $(OBJ_DIR)/src/matrixMath/MatrixTransform.o \
	$(OBJ_DIR)/src/scene/Camera.o $(OBJ_DIR)/src/scene/Transformation.o
DEP = $(OBJ:.o=.d) $(TEST_OBJ:.o=.d)

all: $(NAME)

check-deps:
	@command -v $(PKG_CONFIG) >/dev/null 2>&1 || { echo "Install pkg-config/pkgconf; see README.md." >&2; exit 1; }
	@$(PKG_CONFIG) --exists glfw3 || { echo "Install GLFW development files; see README.md." >&2; exit 1; }

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(GLAD_INCLUDE) $(GLFW_INCLUDE) $(OBJ) -o $(NAME) $(GLFW_LIB)
	@printf "$(GREEN)Compiled$(RESET)\n"

test: $(TEST_NAMES)
	./$(OBJ_DIR)/tests/testMatrixTransform
	./$(OBJ_DIR)/tests/testMatrixStack
	./$(OBJ_DIR)/tests/testUnitCube
	./$(OBJ_DIR)/tests/testArm
	./$(OBJ_DIR)/tests/testHumanoid

$(OBJ_DIR)/tests/testMatrixTransform: $(OBJ_DIR)/tests/testMatrixTransform.o $(TEST_SUPPORT)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/tests/testMatrixStack: $(OBJ_DIR)/tests/testMatrixStack.o $(OBJ_DIR)/src/matrixMath/MatrixStack.o $(OBJ_DIR)/src/matrixMath/MatrixTransform.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/tests/testUnitCube: $(OBJ_DIR)/tests/testUnitCube.o $(OBJ_DIR)/src/render/UnitCube.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/tests/testArm: $(OBJ_DIR)/tests/testArm.o $(OBJ_DIR)/src/scene/Arm.o $(OBJ_DIR)/src/matrixMath/MatrixStack.o $(OBJ_DIR)/src/matrixMath/MatrixTransform.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/tests/testHumanoid: $(OBJ_DIR)/tests/testHumanoid.o $(OBJ_DIR)/src/scene/Humanoid.o $(OBJ_DIR)/src/scene/Arm.o $(OBJ_DIR)/src/matrixMath/MatrixStack.o $(OBJ_DIR)/src/matrixMath/MatrixTransform.o
	$(CXX) $(CXXFLAGS) $^ -o $@

test-render: $(TEST_RENDER)
	./$(TEST_RENDER)

$(TEST_RENDER): $(TEST_RENDER).o $(filter-out $(OBJ_DIR)/src/main.o,$(OBJ))
	$(CXX) $(CXXFLAGS) $^ $(GLFW_LIB) -o $@

$(OBJ_DIR)/%.o: %.cpp Makefile | $(OBJ_DIR) check-deps
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(GLAD_INCLUDE) $(GLFW_INCLUDE) -c $< -o $@

-include $(DEP)

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -f $(NAME)
	@printf "$(GREEN)Cleaned$(RESET)\n"

re:
	$(MAKE) fclean
	$(MAKE) all

valgrind: $(NAME)
	valgrind --leak-check=full \
			 --show-leak-kinds=definite \
			 --log-file=valgrind-out.txt \
		 ./$(NAME) models/cube.obj textureSources/wood.bmp

.PHONY: all test test-render check-deps clean fclean re valgrind
