#include "TerminalApp.hpp"

#include <cctype>
#include <filesystem>
#include <memory>
#include <system_error>
#include <utility>

#include "plot/AgriculturalPlot.hpp"
#include "plot/CommercialPlot.hpp"
#include "plot/ResidentialPlot.hpp"

namespace {

constexpr const char *kDatabaseFile = "database.txt";

} // namespace

void TerminalApp::loadDatabase(std::ostream &out) {
  std::error_code ec;
  if (!std::filesystem::exists(kDatabaseFile, ec))
    return;

  try {
    LandRegistry replacement;
    replacement.load(kDatabaseFile);
    registry_ = std::move(replacement);
    out << "База загружена из " << kDatabaseFile << " (" << registry_.size()
        << " участков)\n";
  } catch (const std::exception &e) {
    out << "файл " << kDatabaseFile << " повреждён и пропущен (" << e.what()
        << ")\n";
  }
}

void TerminalApp::saveDatabase(std::ostream &out) {
  if (!dirty_)
    return;
  try {
    registry_.save(kDatabaseFile);
    dirty_ = false;
    out << "База сохранена в " << kDatabaseFile << " (" << registry_.size()
        << " участков).\n";
  } catch (const std::exception &e) {
    out << "ОШИБКА СОХРАНЕНИЯ: " << e.what() << '\n';
  }
}

void TerminalApp::run(std::istream &in, std::ostream &out) {
  out_ = &out;
  out << "Терминал управления базой земельных участков\n"
      << "'help' - список команд, '<команда> --help' - справка, 'exit' - "
         "выход\n\n";
  loadDatabase(out);

  std::string line;
  while (running_) {
    out << "land_db> " << std::flush;
    if (!std::getline(in, line))
      break;
    execute(tokenize(line), out);
  }
  saveDatabase(out);
}

int TerminalApp::execute(const std::vector<std::string> &tokens,
                         std::ostream &out) {
  if (tokens.empty())
    return 0;
  out_ = &out;

  const std::string &command = tokens.front();
  if (command == "exit" || command == "quit" || command == "q") {
    running_ = false;
    return 0;
  }
  std::vector<std::string> args = tokens;
  if (command == "help" || command == "?")
    args[0] = "--help";

  Args parsed;
  CLI::App app("Терминал управления базой земельных участков");
  configure(app, parsed);

  std::vector<const char *> argv;
  argv.reserve(args.size() + 1);
  argv.push_back("land_db");
  for (const auto &arg : args)
    argv.push_back(arg.c_str());

  try {
    app.parse(static_cast<int>(argv.size()), argv.data());
    return 0;
  } catch (const CLI::ParseError &e) {
    return app.exit(e, out, out);
  } catch (const std::exception &e) {
    out << "Ошибка: " << e.what() << '\n';
    return 1;
  }
}

void TerminalApp::configure(CLI::App &app, Args &args) {
  app.require_subcommand(1);
  addCommandsAdd(app, args);
  addCommandsView(app, args);
  addCommandsEdit(app, args);

  auto *seed =
      app.add_subcommand("seed", "Наполнить базу демонстрационными участками");
  seed->callback([this] { seedDemoData(); });
}

void TerminalApp::addCommonOptions(CLI::App *sub, Args &args) {
  sub->add_option("cadastral", args.cadastral, "Кадастровый номер")->required();
  sub->add_option("-a,--area", args.area, "Площадь")
      ->required()
      ->check(CLI::Range(0.0, 1.0e9));
  sub->add_option("-s,--address", args.address, "Адрес")->required();
  sub->add_option("-o,--owner", args.fullName, "ФИО владельца")->required();
  sub->add_option("-p,--passport", args.passport, "Паспорт владельца")
      ->required();
  sub->add_option("-t,--phone", args.phone, "Телефон");
}

void TerminalApp::addCommandsAdd(CLI::App &app, Args &args) {
  auto *add = app.add_subcommand("add", "Добавить участок в базу");
  add->require_subcommand(1);

  auto *res = add->add_subcommand("residential", "Участок под ИЖС");
  addCommonOptions(res, args);
  res->add_option("-H,--house", args.houseArea, "Площадь дома")
      ->check(CLI::Range(0.0, 1.0e9))
      ->capture_default_str();
  res->callback([this, &args] {
    registry_.add(std::make_unique<ResidentialPlot>(
        args.cadastral, args.area, args.address,
        Owner{args.fullName, args.passport, args.phone}, args.houseArea));
    dirty_ = true;
    *out_ << "Добавлен: " << *registry_.find(args.cadastral) << '\n';
  });

  auto *agro =
      add->add_subcommand("agricultural", "Сельскохозяйственный участок");
  addCommonOptions(agro, args);
  agro->add_option("-c,--crop", args.crop, "Культура")->required();
  agro->add_option("-f,--fertility", args.fertility,
                   "Коэффициент плодородия [0.1; 1.0]")
      ->check(CLI::Range(0.1, 1.0))
      ->capture_default_str();
  agro->callback([this, &args] {
    registry_.add(std::make_unique<AgriculturalPlot>(
        args.cadastral, args.area, args.address,
        Owner{args.fullName, args.passport, args.phone}, args.crop,
        args.fertility));
    dirty_ = true;
    *out_ << "Добавлен: " << *registry_.find(args.cadastral) << '\n';
  });

  auto *com = add->add_subcommand("commercial", "Коммерческий участок");
  addCommonOptions(com, args);
  com->add_option("-u,--purpose", args.purpose,
                  "Назначение (магазин, офис, ...)")
      ->required();
  com->add_option("-n,--floors", args.floors, "Этажность")
      ->check(CLI::Range(1, 200))
      ->capture_default_str();
  com->callback([this, &args] {
    registry_.add(std::make_unique<CommercialPlot>(
        args.cadastral, args.area, args.address,
        Owner{args.fullName, args.passport, args.phone}, args.purpose,
        args.floors));
    dirty_ = true;
    *out_ << "Добавлен: " << *registry_.find(args.cadastral) << '\n';
  });
}

void TerminalApp::addCommandsView(CLI::App &app, Args &args) {
  auto *list = app.add_subcommand("list", "Показать все участки базы");
  list->callback([this] {
    registry_.forEach(
        [this](const LandPlot &plot) { *out_ << "  - " << plot << '\n'; });
    *out_ << "Всего участков: " << registry_.size() << '\n';
  });

  auto *find = app.add_subcommand("find", "Найти участок по номеру");
  find->add_option("cadastral", args.cadastral, "Кадастровый номер")
      ->required();
  find->callback([this, &args] {
    if (const LandPlot *plot = registry_.find(args.cadastral))
      *out_ << "Найден: " << *plot << '\n';
    else
      *out_ << "Участок " << args.cadastral << " не найден.\n";
  });

  auto *tax = app.add_subcommand("tax", "Налог участка по номеру");
  tax->add_option("cadastral", args.cadastral, "Кадастровый номер")->required();
  tax->callback([this, &args] {
    if (const LandPlot *plot = registry_.find(args.cadastral))
      *out_ << "Налог [" << plot->categoryName()
            << "]: " << plot->calculateTax() << " руб.\n";
    else
      *out_ << "Участок " << args.cadastral << " не найден.\n";
  });

  auto *total = app.add_subcommand("total", "Суммарный налог по всей базе");
  total->callback([this] {
    *out_ << "Суммарный налог по базе (" << registry_.size()
          << " участков): " << registry_.totalTax() << " руб.\n";
  });
}

void TerminalApp::addCommandsEdit(CLI::App &app, Args &args) {
  auto *remove = app.add_subcommand("remove", "Удалить участок по номеру");
  remove->add_option("cadastral", args.cadastral, "Кадастровый номер")
      ->required();
  remove->callback([this, &args] {
    if (registry_.remove(args.cadastral)) {
      dirty_ = true;
      *out_ << "Участок " << args.cadastral << " удалён.\n";
    } else {
      *out_ << "Участок " << args.cadastral << " не найден.\n";
    }
  });

  auto *setArea = app.add_subcommand("set-area", "Изменить площадь участка");
  setArea->add_option("cadastral", args.cadastral, "Кадастровый номер")
      ->required();
  setArea->add_option("area", args.newArea, "Новая площадь, m2")
      ->required()
      ->check(CLI::Range(0.0, 1.0e9));
  setArea->callback([this, &args] {
    if (LandPlot *plot = registry_.find(args.cadastral)) {
      plot->setArea(args.newArea);
      dirty_ = true;
      *out_ << "Обновлён: " << *plot << '\n';
    } else {
      *out_ << "Участок " << args.cadastral << " не найден.\n";
    }
  });
}

void TerminalApp::seedDemoData() {
  const std::string phone = "+7 800 555 35 35";
  int added = 0;
  auto tryAdd = [this, &added](std::unique_ptr<LandPlot> plot) {
    registry_.add(std::move(plot));
    ++added;
  };

  tryAdd(std::make_unique<ResidentialPlot>(
      "38:36:240301:101", 800.0, "г. Городуйск, ул. Садовая, 1",
      Owner{"Иванов Иван Иванович", "9111 555344", phone}, 120.0));
  tryAdd(std::make_unique<ResidentialPlot>(
      "38:36:240301:102", 600.0, "г. Городуйск, ул. Садовая, 2",
      Owner{"Петрова Анна Сергеевна", "9112 663355", phone}, 0.0));
  tryAdd(std::make_unique<AgriculturalPlot>(
      "38:36:240302:201", 15000.0, "Городуйский р-н, д. Дыра, поле №3",
      Owner{"АО «Нива»", "9113 774466", phone}, "пшеница", 0.9));
  tryAdd(std::make_unique<CommercialPlot>(
      "38:36:240303:301", 2500.0, "г. Городуйск, пр. Ленина, 10",
      Owner{"ООО «Паль»", "9114 885577", phone}, "торговый центр", 3));

  if (added > 0)
    dirty_ = true;
  *out_ << "Добавлено демо-участков: " << added << "\n";
}

std::vector<std::string> TerminalApp::tokenize(const std::string &line) {
  std::vector<std::string> tokens;
  std::string current;
  char quote = 0;
  for (char c : line) {
    if (quote != 0) {
      if (c == quote)
        quote = 0;
      else
        current += c;
    } else if (c == '"' || c == '\'') {
      quote = c;
    } else if (std::isspace(static_cast<unsigned char>(c)) != 0) {
      if (!current.empty()) {
        tokens.push_back(std::move(current));
        current.clear();
      }
    } else {
      current += c;
    }
  }
  if (!current.empty())
    tokens.push_back(std::move(current));
  return tokens;
}
