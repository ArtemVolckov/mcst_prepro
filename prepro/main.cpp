#include <prepro/lexer.hpp>
#include <prepro/parser.hpp>
#include <prepro/ast.hpp>
#include <prepro/preprocessor.hpp>
#include <boost/program_options.hpp>

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <memory>
#include <exception>

namespace po = boost::program_options;

int main(int argc, char *argv[]) {
  try {
    po::options_description desc("Параметры:");
    desc.add_options()
      ("help,h", "справка")
      ("version,v", "версия")
      ("input,i", po::value<std::string>(), "входной файл")
      ("output,o", po::value<std::string>(), "выходной файл");

    // позиционные аргументы
    po::positional_options_description positional;
    positional.add("input", 1);
    po::variables_map vm;

    po::store(
      po::command_line_parser(argc, argv)
        .options(desc)
        .positional(positional)
        .run(),
      vm
    );
    po::notify(vm);

    if (vm.contains("help")) {
      std::cout
        << "Препроцессор текста prepro\n\n"
        << "Использование:\n"
        << "  pp [параметры] [входной_файл]\n\n"
        << desc;
      return 0;
    }
    if (vm.contains("version")) {
      std::cout << "prepro 1.0\n";
      return 0;
    }
    std::string src;

    if (vm.contains("input")) {
      std::ifstream file(vm["input"].as<std::string>());
      if (!file) {
        std::cerr << "Не удалось открыть входной файл\n";
        return 1;
      }
      src.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
    else {
      src.assign(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
    }

    prepro::Lexer lexer(src);
    prepro::Parser parser(lexer.tokenize());
    auto root = parser.parse();
    prepro::Preprocessor preprocessor;
    std::string res = preprocessor.process(*root);

    if (vm.contains("output")) {
      std::ofstream out(vm["output"].as<std::string>());
      if (!out) {
        std::cerr << "Не удалось открыть выходной файл\n";
        return 1;
      }
      out << res;
    }
    else {
      std::cout << res;
    }
  }
  catch (const po::error &e) {
    std::cerr << "Ошибка в параметрах командной строки: "
              << e.what() << '\n';
    return 1;
  }
  catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
  return 0;
}