/*
 * file_writer.h
 *
 *  Created on: Feb 7, 2018
 *      Author: Nathan
 */

#ifndef FILE_WRITER_H_
#define FILE_WRITER_H_

enum task_folder {
	FOLDER_PAYLOAD,
	FOLDER_HOUSEKEEP,
};

struct file_queue_message {
	int id;
	// the name of the file
	char file_name[20];
	// the actual data being written
	char *data;
	// the size of the data in the pointer
	int size;
};

void task_file_writer(void *arg);

#endif /* FILE_WRITER_H_ */
