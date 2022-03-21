#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <boost/program_options.hpp>
#include "string.hpp"
using namespace std;
using namespace std::filesystem;
using namespace boost::program_options;

struct stat_info
{
	int modified_lines;
	int deleted_lines;
	int failed_lines;
	int skipped_lines;

	int modified_files;
	int failed_files;
	int skipped_files;
};

// AutoDock4 atom types, e.g. H, HD, C, A.
const set<string> adtypes
{
	"H" , //  0
	"HD", //  1
	"C" , //  2
	"A" , //  3
	"N" , //  4
	"NA", //  5
	"OA", //  6
	"S" , //  7
	"SA", //  8
};

const set<char> adtypes1
{
	'P' , // 10
	'F' , // 11
	'I' , // 14
	'K' , // 22
	'U' , // 29
	'B' , // 33
};

const set<string> adtypes2
{
	"Se", "Cl", "Br", "Zn", "Fe", "Mg", "Ca", "Mn", "Cu", "Na",
	"Hg", "Ni", "Co", "Cd", "As", "Sr", "Cs", "Mo", "Si", "Pt",
	"Li", "Sb", "Bi", "Ti", "Sn", "Ra", "Tl", "Cr", 
};

const set<char> elems1
{
	'H', 'B', 'C', 'N', 'O', 'F', 'P', 'S', 'K', 'V', 'Y', 'I', 'W', 'U',
};

const set<string> elems2
{
	"He", "Li", "Be", "Ne", "Na", "Mg", "Al", "Si", "Cl", "Ar",
	"Ca", "Sc", "Ti", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn",
	"Ga", "Ge", "As", "Se", "Br", "Kr", "Rb", "Sr", "Zr", "Nb",
	"Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn", "Sb",
	"Te", "Xe", "Cs", "Ba", "Hf", "Ta", "Re", "Os", "Ir", "Pt",
	"Au", "Hg", "Tl", "Pb", "Bi", "Po", "At", "Rn", "Fr", "Ra",
	"Rf", "Db", "Sg", "Bh", "Hs", "Mt", "Ds", "Rg", "Cn", "Nh",
	"Fl", "Mc", "Lv", "Ts", "Og", "La", "Ce", "Pr", "Nd", "Pm",
	"Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb", "Lu",
	"Ac", "Th", "Pa", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es",
	"Fm", "Md", "No", "Lr",
};

bool parse_elem(string& elem, string& res, string& out)
{
	bool fixed = false;
	if (isspace(elem[0]) || isdigit(elem[0]))
	{
		elem[1] = toupper(elem[1]);
		if (adtypes1.count(elem[1]))
		{
			out = elem[1];
			fixed = true;
		}
		else if (elem[1] == 'O')
		{
			out = "OA";
			fixed = true;
		}
		else if (elem[1] == 'N')
		{
			if (res == "LYS" || res == "ARG")
			{
				out = "NA";
				fixed = true;
			}
			else if (res == "ASN" || res == "GLN")
			{
				out = 'N';
				fixed = true;
			}
			else
			{
				out = 'N';
			}
		}
		else if (elem[1] == 'C' || elem[1] == 'H' || elem[1] == 'S')
		{
			out = elem[1];
		}
		else if (elem[1] == 'V' || elem[1] == 'B') // warning AutoDock Vina doesn't support V or B
		{
			out = elem[1];
			fixed = true;
		}
		else
		{
			out = elem[1];
		}
	}
	else
	{
		elem[0] = toupper(elem[0]);
		elem[1] = tolower(elem[1]);
		out = elem;
		if (adtypes2.count(elem))
		{
			fixed = true;
		}
	}

	return fixed;
}

bool fix(istream& istream, ostream& ostream, stat_info& si, const string& filename)
{
	string line;
	bool first = true, err = false, ok = false;

	for (int ln = 1; safe_getline(istream, line); ln++)
	{
		auto sig = line.substr(0, 6);

		if (line.length() >= 78 && (sig == "ATOM  " || sig == "HETATM" || sig == "ANISOU"))
		{
			string elem1 = line.substr(12, 2), res = line.substr(17, 3), elem2 = trim_end(line.substr(77, 2));
			if (elem2 == "?")
			{
				if (parse_elem(elem1, res, elem2))
				{
					line.resize(77);
					line += elem2;
					si.modified_lines++;
					ok = true;
				}
				else
				{
					si.failed_lines++;
					cerr << "ERROR: unsupported element " << elem2 << " on line " << ln;
					if (filename.size())
						cerr << " in " << filename << endl;
					err = true;
				}
			}
		}

		if (first)
			first = false;
		else
			ostream << endl;
		ostream << line;
	}

	if (err)
		si.failed_files++;
	else if (ok)
		si.modified_files++;
	else
		si.skipped_files++;

	return ok;
}

int main(int argc, char* argv[])
{
	const bool default_inplace = false;
	const string default_output = "@@/@_fixed.pdbqt";

	vector<path> inputs;
	string output;
	bool inplace;

	try
	{
		options_description options("Options");
		options.add_options()
			("input-files,i", value<vector<path>>(&inputs), "one or more pdbqt files to be fixed")
			("inplace,a", bool_switch(&inplace)->default_value(default_inplace), "fix pdbqt file inplace without generating new files (will ignore --output argument)")
			("output,o", value<string>(&output)->default_value(default_output), "output file name template, supported substitutions includes:\n{name} or @ indicating the original file name (no extension)\n{path} or @@ indicating the original file full path (no extension, no trailing slash)")
			("help", "this help information")
			("version", "version information");

		positional_options_description positional;
		positional.add("input-files", -1);

		variables_map vm;
		store(command_line_parser(argc, argv).options(options).positional(positional).run(), vm);
		notify(vm);

		if (vm.count("help")) // program input
		{
			cout << "Usage: " << argv[0] << " <input-file1> [<input-file2> [<input-file3> [...]]] [options]" << endl;
			cout << "pdbqt element fixing tool by Maozi Chen" << endl;
			cout << "This program fixes pdbqt files by inferring AutoDock Vina element types for elements marked as unknown with a question mark (?)." << endl;
			cout << endl;
			cout << options;
			return 0;
		}

		if (vm.count("version"))
		{
			cout << "1.0.1 (2021-05-21)" << endl;
			return 0;
		}

		stat_info si{};
		if (vm.count("input-files"))
		{
			for (auto& input : inputs)
			{
				if (!inplace)
				{
					string outfile = output;

					// substitute {path} or @@
					auto par = input.parent_path().string();
					if (par.size() && par.back() == '/')
						par.pop_back();

					auto pos = outfile.find("{path}");
					if (pos == string::npos)
					{
						pos = outfile.find("@@");
						if (pos != string::npos)
							outfile.replace(pos, 2, par);
					}
					else
					{
						outfile.replace(pos, 6, par);
					}

					// substitute {name} or @
					auto name = input.filename().stem().string();

					pos = outfile.find("{name}");
					if (pos == string::npos)
					{
						pos = outfile.find('@');
						if (pos != string::npos)
							outfile.replace(pos, 1, name);
					}
					else
					{
						outfile.replace(pos, 6, name);
					}

					ofstream ofs(outfile);

					if (outfile == input)
					{
						inplace = true;
					}
					else
					{
						ifstream ifs(input);
						fix(ifs, ofs, si, input.string());
						cerr << "INFO: written to file '" << outfile << '\'' << endl;
					}
				}

				if (inplace)
				{
					ifstream ifs(input);
					path tmp = temp_directory_path() / (to_string(rand()) + input.filename().string());
					ofstream ofs(tmp);

					if (fix(ifs, ofs, si, input.string()))
					{
						ifs.close();
						ofs.close();
						copy(tmp, input, copy_options::overwrite_existing);
						cerr << "INFO: written to file '" << input << '\'' << endl;
					}
					else
					{
						ifs.close();
						ofs.close();
						cerr << "INFO: skipped failed file '" << input << '\'' << endl;
					}
					remove(tmp);
				}
			}
		}
		else
		{
			fix(cin, cout, si, "");
		}

		cerr << "INFO: fixed " << si.modified_lines << " lines, untouched " << si.skipped_lines << " lines, error " << si.failed_lines << " lines" << endl;
		cerr << "INFO: fixed " << si.modified_files << " files, untouched " << si.skipped_files << " files, error " << si.failed_files << " files" << endl;
		return 0;
	}
	catch (exception& ex)
	{
		cerr << "ERROR: " << ex.what() << endl;
		return 2;
	}
}
