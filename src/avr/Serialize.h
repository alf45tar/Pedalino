void serialize_lcd1(char *l) {

  // Memory pool for JSON object tree.
  //
  StaticJsonBuffer<200> jsonBuffer;

  // StaticJsonBuffer allocates memory on the stack, it can be
  // replaced by DynamicJsonBuffer which allocates in the heap.
  //
  // DynamicJsonBuffer  jsonBuffer(200);

  // Create the root of the object tree.
  //
  JsonObject& root = jsonBuffer.createObject();

  // Add values in the object
  //
  root["lcd1"] = l;

  Serial3.write(0xF0);
  root.printTo(Serial3);
  Serial3.write(0xF7);
}


void serialize_lcd2(char *l) {

  // Memory pool for JSON object tree.
  //
  StaticJsonBuffer<200> jsonBuffer;

  // StaticJsonBuffer allocates memory on the stack, it can be
  // replaced by DynamicJsonBuffer which allocates in the heap.
  //
  // DynamicJsonBuffer  jsonBuffer(200);

  // Create the root of the object tree.
  //
  JsonObject& root = jsonBuffer.createObject();

  // Add values in the object
  //
  root["lcd2"] = l;

  Serial3.write(0xF0);
  root.printTo(Serial3);
  Serial3.write(0xF7);
}
