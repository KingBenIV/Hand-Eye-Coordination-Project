#include <stdio.h>
#include <stdlib.h>
#include <libserialport.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <iostream>
#include <cstdlib>

#define BAUD 9600

#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <vector>
#include <stdio.h>

using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
	/*Variables for camera function*/
	int n = 0;
	char filename[200];
	string window_name = "video | q or esc to quit";
	Mat frame;
	/*Setup camera and check for camera*/
	namedWindow(window_name);
	VideoCapture cap(0);
	if (!cap.isOpened()) {

		cout << "cannot open camera";

	}

	/*Variables for serial comms */
	struct sp_port* port;
	int err;
	int key = 0;
	char cmd; //cmd sent to box
	int cmd_1 = 0; //cmd for firts cup
	int cmd_2 = 0; //cmd for second cup
	int cmd_3 = 0; //cmd for third cup

	/* Set up and open the port */
	/* check port usage */
	if (argc < 2)
	{
		/* return error */
		fprintf(stderr, " Port use\n");
		exit(1);
	}

	/* get port name */
	err = sp_get_port_by_name(argv[1], &port);
	if (err == SP_OK)
		/* open port */
		err = sp_open(port, SP_MODE_WRITE);
	if (err != SP_OK)
	{
		/* return error */
		fprintf(stderr, " Can't open port %s\n", argv[1]);
		exit(2);
	}

	/* set Baud rate */
	sp_set_baudrate(port, BAUD);
	/* set the number of bits */
	sp_set_bits(port, 8);


	/* set up to exit when q key is entered */
	while (key != 'q') {
		cap >> frame;
		//Find the BRG values of the three pixels at the centre of the three zones
		Vec3b intensity_1 = frame.at<Vec3b>(175, 300);
		Vec3b intensity_2 = frame.at<Vec3b>(175, 375);
		Vec3b intensity_3 = frame.at<Vec3b>(175, 435);

		///////////////* Determining which cmds to send*///////////////

		//where should the first cup go
		if (intensity_1.val[0] > 200) { //is the object blue
			cmd_1 = 0b00000011; //binary for 3
		}
		else if (intensity_1.val[1] > 190) { // object is green
			cmd_1 = 0b00000001; //binary for 1
		}
		else if (intensity_1.val[2] > 190) { // object is red
			cmd_1 = 0b00000010; //binary for 2
		}

		//where should the second cup go
		if (intensity_2.val[0] > 200) { //is the object blue
			cmd_2 = 0b00001100; //binary for 12
		}
		else if (intensity_2.val[1] > 170) { // object is green
			cmd_2 = 0b00000100; //binary for 4
		}
		else if (intensity_2.val[2] > 190) { // object is red
			cmd_2 = 0b00001000; //binary for 8
		}

		//where should the thrid cup go
		if (intensity_3.val[0] > 128) { //is the object blue
			cmd_3 = 0b00110000; //binary for 48
		}
		else if (intensity_3.val[1] > 128) { // object is green
			cmd_3 = 0b00010000; //binary for 16
		}
		else if (intensity_3.val[2] > 128) { // object is red
			cmd_3 = 0b00100000; //binary for 32
		}

		cmd = cmd_1 + cmd_2 + cmd_3;

		////////////* End of 'Determining which cmds to send' *////////////


		////////////*The code contained here modifies the output pixel values*////////////
		for (int i = 150; i < 200; i++)
		{
			for (int j = 275; j < 325; j++)
			{
				/*The following lines make the green and blue channels zero
				(this section of the image will be shades of red)*/
				frame.at<Vec3b>(i, j)[0] = 0;
				frame.at<Vec3b>(i, j)[1] = 0;
			}
		}
		for (int i = 150; i < 200; i++)
		{
			for (int j = 350; j < 400; j++)
			{
				/*The following lines make the red and blue channels zero
				(this section of the image will be shades of green)*/
				frame.at<Vec3b>(i, j)[0] = 0;
				frame.at<Vec3b>(i, j)[2] = 0;
			}
		}
		for (int i = 150; i < 200; i++)
		{
			for (int j = 410; j < 460; j++)
			{
				/*The following lines make the red and green channels zero
				(this section of the image will be shades of blue)*/
				frame.at<Vec3b>(i, j)[1] = 0;
				frame.at<Vec3b>(i, j)[2] = 0;
			}
		}
		///////////////*End of modifying pixel values*///////////////

		imshow(window_name, frame);
		char key = (char)waitKey(25);
		/* write the number "cmd" to the port */
		sp_blocking_write(port, &cmd, 1, 100);

		switch (key) {
		case 'q':
		case 'Q':
		case 27: //escape key
			return 0;
		case ' ': //Save an image
			sprintf_s(filename, "filename%.3d.jpg", n++);
			imwrite(filename, frame);
			cout << "Saved " << filename << endl;
			break;
		default:
			break;
		}

		Sleep(100);
	}
	/* close the port */
	sp_close(port);
	return 0;
}