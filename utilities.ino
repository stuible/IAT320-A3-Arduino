//Returns average in float array, taken from https://forum.arduino.cc/index.php?topic=232508.0
float arrayAverage (float theArray[], int len)  // assuming array is int.
{
  float sum = 0;
  for (int i = 0 ; i < len ; i++) {
    sum += theArray[i] ;
  }
  return  ((float) sum) / len ;  // average will be fractional, so float may be appropriate.
}

//Returns average in float array, ignoring 0 values (Inspired by function above)
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
