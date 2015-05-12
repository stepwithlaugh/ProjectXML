#include <iostream>
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include "tinyxml/tinyxml.h"
#include "tinyxml/tinyxmlerror.cpp"
#include "tinyxml/tinyxmlparser.cpp"
#include "md5.h"

using namespace std;
namespace fs = boost::filesystem;

struct Fileinfo {
	string path;
	string hash;
	int size;
	string flag = "NEW";
};

vector<Fileinfo> compare_lists(vector<Fileinfo> newfl, vector<Fileinfo> oldfl) {
	for (vector<Fileinfo>::iterator itnew = newfl.begin(); itnew < newfl.end(); itnew++) {

		for (vector<Fileinfo>::iterator itold = oldfl.begin(); itold < oldfl.end(); itold++) {
			if ((itnew->path == itold->path) && (itnew->hash == itold->hash)) {
				itnew->flag = "UNCHANGED";
				oldfl.erase(itold);
				break;
			}
			if ((itnew->path == itold->path) && (itnew->hash != itold->hash)) {
				itnew->flag = "CHANGED";
				oldfl.erase(itold);
				break;
			}
		}
	}
	for (vector<Fileinfo>::iterator itold = oldfl.begin(); itold < oldfl.end(); itold++) {
		itold->flag = "DELETED";
		newfl.push_back(*itold);
	}
	return newfl;
}

void save2xml(string filename, vector<Fileinfo> vec_finfo) {
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(decl);
	for (Fileinfo it : vec_finfo) {
		TiXmlElement * element = new TiXmlElement("File");
		doc.LinkEndChild(element);
		element->SetAttribute("path", it.path.c_str());
		element->SetAttribute("size", it.size);
		element->SetAttribute("hash", it.hash.c_str());
		//element->SetAttribute("flag", it.flag);
		TiXmlText * text = new TiXmlText("");
		element->LinkEndChild(text);
	}
	doc.SaveFile(filename.c_str());
}

void loadxml(string filename, vector<Fileinfo> & vec_finfo) {
	Fileinfo it;
	TiXmlDocument doc;
	doc.LoadFile(filename.c_str());
	TiXmlHandle docHandle(&doc);
	TiXmlElement* child = docHandle.FirstChild("File").ToElement();
	for (child; child; child = child->NextSiblingElement())
	{

		it.size = atoi(child->Attribute("size"));
		it.hash = child->Attribute("hash");
		it.path = child->Attribute("path");
		vec_finfo.push_back(it);
	}
}

void get_dir_list(fs::directory_iterator iterator, vector<Fileinfo> & vec_finfo, Fileinfo & finfo, ifstream & ifs) {
	for (; iterator != fs::directory_iterator(); ++iterator)
	{
		if (fs::is_directory(iterator->status())) {
			fs::directory_iterator sub_dir(iterator->path());
			get_dir_list(sub_dir, vec_finfo, finfo, ifs);

		}
		else
		{

			finfo.path = iterator->path().string();
			replace(finfo.path.begin(), finfo.path.end(), '\\', '/');
			finfo.size = fs::file_size(iterator->path());
			ifs.open(finfo.path, ios_base::binary);
			string content((istreambuf_iterator<char>(ifs)),
				(istreambuf_iterator<char>()));
			finfo.hash = md5(content);
			ifs.close();
			vec_finfo.push_back(finfo);
		}

	}
}


void print_finfo_vec(vector<Fileinfo> vec) {
	for (Fileinfo element : vec) {
		cout << element.path << endl <<
			element.size << endl <<
			element.hash << endl <<
			element.flag << endl << "-------" << endl;
	}
}

// 

int main() {
	ofstream myfile;
	string path, dirpath;
	Fileinfo finfo;
	ifstream ifs;
	string checkstatus;
	cout << "Do you wish to save filelist or check current folder with previous result?" <<
		endl << "(check/save or anything else for neither)" << endl;
	getline(cin, checkstatus);
	cout << "Folder path:" << endl;
	getline(cin, path);
	vector<Fileinfo> vec_finfo;
	vector<Fileinfo> vec_finfo_old;
	try {
		fs::directory_iterator home_dir(path);
		get_dir_list(home_dir, vec_finfo, finfo, ifs);
	}
	catch (const boost::filesystem::filesystem_error& e) {
		cout << "INVALID PATH" << endl;
		checkstatus = "null";
	}
	if (checkstatus == "save") {
		save2xml("example.xml", vec_finfo);
		print_finfo_vec(vec_finfo);
	}
	if (checkstatus == "check") {
		loadxml("example.xml", vec_finfo_old);
		print_finfo_vec(compare_lists(vec_finfo, vec_finfo_old));
	}
	if ((checkstatus != "save") && (checkstatus != "check")) {
		print_finfo_vec(vec_finfo);
	}
	return 0;
}
