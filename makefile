plot_lattice: plot_lattice.cpp find_orientation.cpp diffraction_data.h
	g++ -o plot_lattice find_orientation.cpp  plot_lattice.cpp -I/Users/yoshi/include  /usr/local/lib/libfltk.a -framework Cocoa /usr/local/lib/libgsl.a /usr/local/lib/libgslcblas.a -lncurses 
