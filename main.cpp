#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include "tinyxml/tinyxml.h"
#include "tinyxml/tinyxmlerror.cpp"
#include "tinyxml/tinyxmlparser.cpp"

using namespace std;
namespace fs = boost::filesystem;
string test;

//������ ����� ���������� � ���������, � ��������� ���������� � ������
struct Fileinfo {
	string path;
	string hash;
	int size;
	char flag;
};

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
		element->SetAttribute("flag", it.flag);
		TiXmlText * text = new TiXmlText("");
		element->LinkEndChild(text);
	}
	doc.SaveFile(filename.c_str());
}

void get_dir_list(fs::directory_iterator iterator, vector<Fileinfo> * vec_finfo) {  //������� ������ ������ � ����� � ����������
	Fileinfo finfo; //���������� ���������, � ������� ����� ���������� ������ � ���������� �� � ������
	for (; iterator != fs::directory_iterator(); ++iterator)
	{
		if (fs::is_directory(iterator->status())) { //���� ���������� �� �����, �� ���������� ��������� ��� �� ������� ��� ���� �����
			fs::directory_iterator sub_dir(iterator->path());
			get_dir_list(sub_dir, vec_finfo);
		}
		else //� ���� ���, �� ���������� � ��������� ���, ������, ���, � ������ (����������� ���� �����, ����� ����� ������ ��������� � ������)
		{
			finfo.path = iterator->path().string();
			finfo.size = fs::file_size(iterator->path());
			finfo.hash = "hash";
			finfo.flag = 'f';
			vec_finfo->push_back(finfo);
		}
	}
}

int main() {
	string path, dirpath;
	cout << "Path:" << endl;
	getline(cin, path);


	cout << endl;
	vector<Fileinfo> vec_finfo; //������ � ������� �� ����� ���������� ������� ����� ���������
	fs::directory_iterator home_dir(path);
	get_dir_list(home_dir, &vec_finfo);

	//������� ������ ������, �������� � �.�.:
	for (Fileinfo element : vec_finfo) {
		cout << element.path << endl <<
			element.size << endl;
	}
	save2xml("example.xml", vec_finfo);  //��������� ���������� � ��� ����� � ������ example.xml, ��������� �� � ����� ��� ��������� main.cpp
	cin.clear();
	fflush(stdin);
	cin.get();
	return 0;
}

