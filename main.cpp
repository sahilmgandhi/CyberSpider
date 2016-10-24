//#include <iostream>
//#include <cmath>
//#include <cassert>
//#include "DiskMultiMap.h"
//#include "MultiMapTuple.h"
//#include <stdio.h>
//
//using namespace std;
//
////int main()
////{
////	DiskMultiMap x;
////	x.createNew("myhashtable.dat", 100); // empty, with 100 buckets
////	x.insert("hmm.exe", "pfft.exe", "m52902");
////	x.insert("hmm.exe", "pfft.exe", "m52902");
////	x.insert("hmm.exe", "pfft.exe", "m10001");
////	x.insert("blah.exe", "bletch.exe", "m0003");
////	DiskMultiMap::Iterator it = x.search("hmm.exe");
////	if (it.isValid())
////	{
////		cout << "I found at least 1 item with a key of hmm.exe\n";
////		do
////		{
////			MultiMapTuple m = *it; // get the association
////			cout << "The key is: " << m.key << endl;
////			cout << "The value is: " << m.value << endl;
////			cout << "The context is: " << m.context << endl;
////			cout << endl;
////			++it; // advance iterator to the next matching item
////		} while (it.isValid());
////	}
////
////	DiskMultiMap::Iterator it1 = x.search("goog.exe");
////	if (!it1.isValid())
////		cout << "the value was not found in the hash table" << endl;
////}
//
////int main()
////{
////	DiskMultiMap x;
////	x.createNew("myhashtable1.dat", 100); // empty, with 100 buckets
////	x.insert("hmm.exe", "pfft.exe", "m52902");
////	x.insert("hmm.exe", "pfft.exe", "m52902");
////	x.insert("hmm.exe", "pfft.exe", "m10001");
////	x.insert("blah.exe", "bletch.exe", "m0003");
////	// line 1
////	if (x.erase("hmm.exe", "pfft.exe", "m52902") == 2)
////		cout << "Just erased 2 items from the table!\n";
////	else
////		cout << " could not find hmm.exe and pfft.exe and m52902 in the file " << endl;
////	// line 2
////	if (x.erase("hmm.exe", "pfft.exe", "m10001") > 0)						// TODO: if there are many items that are deleted from the table, then when the last one left is deleted, you need to kill the key
////		cout << "Just erased at least 1 item from the table!\n";
////	DiskMultiMap::Iterator it3 = x.search("hmm.exe");
////	if (it3.isValid())
////		cout << "found some Hmm" << endl;
////	else
////		cout << " couldnt find it properly slkjflsadjfjsdfj" << endl;
////	// line 3
////	if (x.erase("blah.exe", "bletch.exe", "m66666") == 0)
////		cout << "I didn't erase this item cause it wasn't there\n";
////	
////	DiskMultiMap y;
////	y.createNew("myhashtable2.dat", 100);
////	y.insert("hmm.exe", "fft", "991");
////	y.insert("hmm.exe", "fft1", "991");
////	y.insert("hmm.exe", "fft1", "991");
////	int a = y.erase("hmm.exe", "fft1", "991");
////	cout << "the number of fft1 erased were: " << a << endl;
////	int b = y.erase("hmm.exe", "fft", "991");
////	cout << "the numer of fft erased were: " << b << endl;
////	DiskMultiMap::Iterator it1 = y.search("hmm.exe");
////	if (it1.isValid())
////		cout << "found hmm.exe";
////	else
////		cout << "could not find this key";
////	y.insert("blargh.exe", "shouldbePut", "motherofGOd");
////}
//
//int main ()
//{
//	DiskMultiMap s;
//	s.createNew("swag.dat", 100);
//
//	s.insert("sahil", "boy", "ee major");
//	s.insert("sahil", "man", "ee grad");
//	s.insert("sahil", "friend", "superb");
//	s.erase("sahil", "friend", "superb");
//	s.insert("prasann", "boy", "faceles");
//
//	DiskMultiMap::Iterator it = s.search("sahil");
//	if (it.isValid())
//	{
//		cout << "I found at least 1 item with a key of sahil.exe\n";
//		do
//		{
//			MultiMapTuple m = *it; // get the association
//			cout << "The key is: " << m.key << endl;
//			cout << "The value is: " << m.value << endl;
//			cout << "The context is: " << m.context << endl;
//			cout << endl;
//			++it; // advance iterator to the next matching item
//		} while (it.isValid());
//	}
//
//	s.close();
//	s.openExisting("swag.dat");
//	s.insert("sahil", "friend", "superb");
//
//	DiskMultiMap::Iterator it1 = s.search("sahil");
//	if (it1.isValid())
//	{
//		cout << "I found at least 1 item with a key of sahil\n";
//		do
//		{
//			MultiMapTuple m = *it1; // get the association
//			cout << "The key is: " << m.key << endl;
//			cout << "The value is: " << m.value << endl;
//			cout << "The context is: " << m.context << endl;
//			cout << endl;
//			++it1; // advance iterator to the next matching item
//		} while (it1.isValid());
//	}
//
//	DiskMultiMap test1;
//	test1.openExisting("testNo1initiator.dat");
//	auto it5 = test1.search("http://taboola.com/ggxr/nrvy/t/");
//	if (it5.isValid())
//	{
//		do
//		{
//			MultiMapTuple m = (*it5); // get the association
//			cout << "The key is: " << m.key << endl;
//			cout << "The value is: " << m.value << endl;
//			cout << "The context is: " << m.context << endl;
//			cout << endl;
//			++it5; // advance iterator to the next matching item
//		} while (it5.isValid());
//	}
//}