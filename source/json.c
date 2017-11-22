#include <stdlib.h>
#include <jansson.h>
#include <string.h>
#include <3ds.h>
#include <stdio.h>

#include "json.h"
#include "ui.h"

#define MAXSIZE 500

int jsonLoad(char* path, s16 selCount, char** selections, u8* selectArray) {
	json_t* root;
	json_error_t err;
	root = json_load_file(path, 0, &err);
	if (!root)
		return -1;

	if (!json_is_object(root)) {
		json_decref(root);
		return -1;
	}

    int position;
	json_t* val = NULL;
    json_t* subval = NULL;
	for (int i = 0; i < selCount; i++) {
        selectArray[i] = 0;
		val = json_object_get(root, selections[i]);
		if (val == NULL)
			continue;
        
        // make it selected, if applicable
        subval = json_object_get(val, "selected");
        if (subval == NULL)
            continue;
		selectArray[i] = (int)json_integer_value(subval);
        
        // rearrange item in array, if applicable
        subval = json_object_get(val, "priority");
        if (subval == NULL)
            continue;
        
        // if the entry is already at this position, just skip
        position = (int)json_integer_value(subval);
        if (i == position)
            continue;
        
        // rearrange
        swapEntries(i, position);
        i--;
	}
    
    json_decref(subval);
	json_decref(val);
	json_decref(root);

    return 0;
}

int jsonSave(char* path, s16 selCount, char** selections, u8* selectArray) {
	FILE* fp = fopen(path, "w+");
	if (fp == NULL)
		return -1;

	fprintf(fp, "{\n");
	for (int i = 0; i < selCount; i++) {
		if (i == selCount - 1)
			fprintf(fp, "\t\"%s\" : { \"selected\" : %d, \"priority\" : %d}\n", selections[i], selectArray[i], i);
		else
			fprintf(fp, "\t\"%s\" : { \"selected\" : %d, \"priority\" : %d},\n", selections[i], selectArray[i], i);
	}
	fprintf(fp, "}");
	fclose(fp);

	return 0;
}