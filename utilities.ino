//Returns average in float array
float arrayAverage (float theArray[], int len)  // assuming array is int.
{
//  Serial.print("len: "); Serial.println(len);
  float sum = 0;  // sum will be larger than an item, long for safety.
  for (int i = 0 ; i < len ; i++) {
//    Serial.print("theArray[i]: "); Serial.println(theArray[i]);
    sum += theArray[i] ;
//    Serial.print("sum: "); Serial.println(sum);
  }
  return  ((float) sum) / len ;  // average will be fractional, so float may be appropriate.
}

//Returns average in float array, ignoring 0 values
float arrayAverageNoZeros (float theArray[], int len)
{
  float sum = 0;
  for (int i = 0 ; i < len ; i++) {
    if(theArray[i] != 0){
      sum += theArray[i] ;
    }
    else len -=1;
  }
  return  ((float) sum) / len ;
}
