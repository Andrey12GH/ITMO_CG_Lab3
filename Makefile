CXX = clang++
CXXFLAGS = -std=gnu++14 -fcolor-diagnostics -fansi-escape-codes -g
TARGET = main
SOURCES = main.cpp own_gl.cpp model.cpp camera.cpp geometry.cpp tgaimage.cpp
OBJECTS = $(SOURCES:.cpp=.o)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean