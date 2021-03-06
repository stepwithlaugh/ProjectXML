#include <iostream>
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include "tinyxml2.h"
#include "tinyxml2.cpp"
#include "md5.h"
#include "sha256.h"
#include "pbfile.pb.h" //файл, который сделал protoc

using namespace std;
namespace fs = boost::filesystem;

struct Fileinfo {
	string path;
	string md5;
	string sha2;
	int size;
	string flag = "NEW";
};

void savepbuf(std::string filename, std::vector<Fileinfo> & vec_finfo) {
	nsofdir::ArrFilep flist;
	nsofdir::Filep * file_entry;
	std::ofstream output(filename, std::ofstream::binary);
	for (Fileinfo it : vec_finfo) {
		//Запись, просто по сделанным методам протобафа
		file_entry = flist.add_filep();
		file_entry->set_filepath(it.path);
		file_entry->set_size(it.size);
		file_entry->set_mdsixhash(it.hash);

	}
	//Вывод файла
	flist.PrintDebugString();
	//Записываем в output файл
	flist.SerializeToOstream(&output);
	output.close();
}

void loadpbuf(std::string filename, std::vector<Fileinfo> & vec_finfo) {
	//вспомогательная структура Fileinfo, через которую заполним вектор
	Fileinfo it;
	//Filelist, в который считаем файл
	nsofdir::ArrFilep flist;  
	nsofdir::Filep file_entry;
	// Открываем наш записанный файл
	std::ifstream input(filename, std::ofstream::binary); 
	//Парсим из файла
	flist.ParseFromIstream(&input);  
	input.close();
	//flist.PrintDebugString(); // Вывод файла
	file_entry.PrintDebugString();
	for (int i = 0; i < flist.filep_size(); i++) {
		file_entry = flist.filep(i);
		it.path = file_entry.filepath();
		it.size = file_entry.size();
		it.hash = file_entry.mdsixhash();
		vec_finfo.push_back(it);
	}
}

vector<Fileinfo> compare_lists(vector<Fileinfo> newfl, vector<Fileinfo> oldfl) {
	for (vector<Fileinfo>::iterator itnew = newfl.begin(); itnew < newfl.end(); itnew++) {

		for (vector<Fileinfo>::iterator itold = oldfl.begin(); itold < oldfl.end(); itold++) {
			if ((itnew->path == itold->path) && (itnew->md5 == itold->md5) && (itnew->sha2 == itold->sha2)) {
				itnew->flag = "UNCHANGED";
				oldfl.erase(itold);
				break;
			}
			if ((itnew->path == itold->path) && (itnew->md5 == itold->md5) || (itnew->sha2 == itold->sha2))  {
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

void saveToXml(string filename, vector<Fileinfo> vec_finfo) {
	XMLDocument doc;
	XMLDeclaration * decl = doc.NewDeclaration();
	doc.LinkEndChild(decl);
	for (Fileinfo it : vec_finfo) {
		XMLElement * element = doc.NewElement("File");
		doc.LinkEndChild(element);
		element->SetAttribute("path", it.path.c_str());
		element->SetAttribute("size", it.size);
		element->SetAttribute("md5", it.md5.c_str());
		element->SetAttribute("sha256", it.sha2.c_str());
		element->SetAttribute("flag", it.flag.c_str());
		XMLText * text = doc.NewText(" ");
		element->LinkEndChild(text);
	}
	doc.SaveFile(filename.c_str());
}

void loadxml(string filename, vector<Fileinfo> & vec_finfo) {
	Fileinfo it;
	XMLDocument doc;
	doc.LoadFile(filename.c_str());
	XMLHandle docHandle(&doc);
	XMLElement* child = docHandle.FirstChildElement("File").ToElement();
	for (child; child; child = child->NextSiblingElement())
	{

		it.size = atoi(child->Attribute("size"));
		it.md5 = child->Attribute("md5");
		it.sha256 = child->Attribute("sha256");
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
			SHA256 sha256;
			finfo.md5 = md5(content);
			finfo.sha2 = sha256(content);
			ifs.close();
			vec_finfo.push_back(finfo);
		}

	}
}


void print_finfo_vec(vector<Fileinfo> vec) {
	for (Fileinfo element : vec) {
		cout << element.path << endl <<
			element.size << endl <<
			element.md5 << endl <<
			element.sha2 << endl <<
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
		endl << "(check/save or anything else for neither)" <<
		"checkpb/savepb for protobuf option" << endl;
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
		if (checkstatus == "savepb") {
		savepbuf("filelist.pb", vec_finfo);
		print_finfo_vec(vec_finfo);
	}
		if (checkstatus == "checkpb") {
		loadpbuf("filelist.pb", vec_finfo_old);
		print_finfo_vec(compare_lists(vec_finfo, vec_finfo_old));
	}
	return 0;
}

