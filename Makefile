# Makefile

# macro
GPP = g++
TARGET = main.o myOpenCV.o GeometricCalibration.o AppearanceEnhancement.o ProCam.o LinearizerOfProjector.o
CV_LIB = -lopencv_core -lopencv_highgui -lopencv_imgproc

#create a.out
a.out: $(TARGET)
	g++ $(TARGET) -o a.out $(CV_LIB)

# .cpp -> .o
.cpp.o:
	g++ -c $<
