/*
 * Storage.h
 *
 *  Created on: 19.05.2017
 *      Author: maro
 */

#ifndef STORAGE_H_
#define STORAGE_H_

//TODO: should serve as wrapper layer for any filesystem, but currently only
//SD or SPIFFS can be used due to double defintion of file.h
#include <FS.h>

class Storage {
public:
	Storage();
	virtual ~Storage();
};

#endif /* STORAGE_H_ */
