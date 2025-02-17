# Compiler và flags
CXX = g++
CXXFLAGS = -c -I"D:\Libraries\SFML-2.6.1\include" -std=c++17 -Wall
LDFLAGS = -L"D:\Libraries\SFML-2.6.1\lib" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network

# Tên đầu ra
TARGET = main.exe

# Tệp nguồn và tệp đối tượng
SOURCES = main.cpp Game.cpp menuCarogame.cpp GameBot.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Luật mặc định
all: $(TARGET)

# Liên kết các đối tượng để tạo file thực thi
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Biên dịch từng tệp nguồn thành tệp đối tượng
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

# Dọn dẹp các tệp tạm thời
clean:
	del /f /q *.o *.exe
