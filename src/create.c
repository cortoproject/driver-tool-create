
#include <driver.tool.create>
#include <corto.util.argparse>

#define CORTO_PACKAGE ("package")
#define CORTO_APPLICATION ("application")

#define CORTO_PROMPT UT_CYAN "corto: " UT_NORMAL

int cortotool_main(int argc, char *argv[]);

static char* cortotool_randomName(void) {
    char buffer[256];

    char *colors[] = {
        "Cayenne",
        "Maroon",
        "Orchid",
        "Magenta",
        "Tangerine",
        "Salmon",
        "Lemon",
        "Clover",
        "Lime",
        "Teal",
        "Turquoise"};

    char *animals[] = {
        "Buffalo",
        "Eagle",
        "Lynx",
        "Porcupine",
        "Lizard",
        "Alpaca",
        "Lemming",
        "Armadillo",
        "Mongoose",
        "Gecko",
        "Beaver",
        "Owl",
        "Cat",
        "Emu",
        "Vulture",
        "Kangaroo",
        "Badger",
        "Hawk",
        "Baboon",
        "Otter",
        "Ibis",
        "Goose",
        "Lemur",
        "Hog",
        "Herring",
        "Sloth",
        "Peacock",
        "Koala",
        "Moose",
        "Tapir"
        };

    strcpy(buffer, colors[rand() % (sizeof(colors) / sizeof(char*))]);
    strcat(buffer, animals[rand() % (sizeof(animals) / sizeof(char*))]);

    return strdup(buffer);
}

static char* cortotool_randomDescription(void) {
    char buffer[256];

    char *function[] = {
        "Car rentals",
        "Ride sharing",
        "Room sharing",
        "Vegan meal delivery",
        "Instant noodles",
        "Premium coffee beans",
        "Air conditioners",
        "Sunscreen",
        "Furniture",
        "A microwave",
        "Conditioner",
        "Grocery stores",
        "Photo sharing",
        "A social network",
        "Coding tutorials",
        "Bodybuilding",
        "Laser hair removal",
        "A movie theater subscription",
        "Plant delivery",
        "Shaving supplies",
        "On demand country music",
        "A dating site",
        "Wine tasting",
        "Solar energy",
        "A towel",
        "A middle out compression algorithm",
        "Putting birds on ",
        "Winter is coming",
        "Space travel",
        "Lightsabers",
        "Hi-speed internet",
        "A web framework",
        "A fitness tracker"};

    char *subject[] = {
        "drones",
        "self driving cars",
        "air travel",
        "kids",
        "millenials",
        "coders",
        "project managers",
        "entrepreneurs",
        "designers",
        "web developers",
        "venture capitalists",
        "sales reps",
        "married couples",
        "wookies",
        "dogs",
        "cats"};

    strcpy(buffer, function[rand() % (sizeof(function) / sizeof(char*))]);
    if (buffer[strlen(buffer) - 1] != ' ') {
        strcat(buffer, " for ");
    }
    strcat(buffer, subject[rand() % (sizeof(subject) / sizeof(char*))]);

    return strdup(buffer);
}

static corto_int16 cortotool_setupProject(
    const char *projectKind,
    const char *dir,
    const char *id,
    const char *name,
    corto_bool isLocal,
    corto_bool isSilent)
{
    CORTO_UNUSED(isLocal);
    CORTO_UNUSED(projectKind);

    if (ut_file_test(dir)) {
        corto_id id;
        sprintf(id, "%s/project.json", dir);
        if (ut_file_test(id)) {
            ut_throw(
                "corto: a project in location '%s' already exists!",
                dir);
            goto error;
        }
    } else if (ut_mkdir(dir)) {
        ut_throw(
            "corto: couldn't create project directory '%s' (check permissions)",
            dir);
        goto error;
    }

    corto_id model_file;
    sprintf(model_file, "%s/model.corto", dir);
    FILE *file = fopen(model_file, "w");
    if (file) {
        fprintf(file, "in %s\n\n", id);
        fprintf(file, "/* Create models for your project in this file\n\n");
        fprintf(file, "class ExampleType {\n");
        fprintf(file, "    // Members\n");
        fprintf(file, "    x: int32\n");
        fprintf(file, "    y: int32\n\n");
        fprintf(file, "    // Constructor and destructor\n");
        fprintf(file, "    construct() int16\n");
        fprintf(file, "    destruct()\n\n");
        fprintf(file, "    // Methods\n");
        fprintf(file, "    add(int32 x, int32 y)\n");
        fprintf(file, "    dot() int32\n");
        fprintf(file, "}\n\n");
        fprintf(file, "*/\n\n");
        fclose(file);
    } else {
        ut_throw(
            "corto: failed to open file '%s' (check permissions)",
            model_file);
        goto error;
    }

    if (!isSilent) {
        printf("\n");
        printf("               \\ /\n");
        printf("              - . -\n");
        printf("               /%s|%s\\\n", UT_GREEN, UT_NORMAL);
        printf("%s                | /\\\n", UT_GREEN);
        printf("%s             /\\ |/\n", UT_GREEN);
        printf("%s               \\|%s\n", UT_GREEN, UT_NORMAL);
        printf ("\nPlanting a new idea in directory %s'%s'%s\n",
            UT_CYAN,
            dir,
            UT_NORMAL);
    }

    return 0;
error:
    return -1;
}

static corto_int16 cortotool_createProjectJson(
    const char *projectKind,
    const char *id,
    const char *dir,
    corto_bool isLocal,
    corto_bool nocoverage,
    corto_string language,
    bool cpp)
{
    FILE *file;
    corto_id buff;
    int8_t count = 0;

    sprintf(buff, "%s/project.json", dir);
    file = fopen(buff, "w");
    if(!file) {
        ut_throw("couldn't create %s/project.json (check permissions)", buff);
        goto error;
    }

    fprintf(file,
        "{\n"\
        "    \"id\": \"%s\",\n"\
        "    \"type\": \"%s\",\n"\
        "    \"value\": {",
        id,
        !strcmp(projectKind, CORTO_PACKAGE) ? "package" : "application");

    char *description = cortotool_randomDescription();

    fprintf(file,  "\n        \"description\": \"%s\"", description);
    fprintf(file, ",\n        \"author\": \"John Doe\"");
    fprintf(file, ",\n        \"version\": \"1.0.0\"");
    fprintf(file, ",\n        \"repository\": null");
    fprintf(file, ",\n        \"license\": null");
    fprintf(file, ",\n        \"language\": \"%s\"", language);
    if (cpp) {
        fprintf(file, ",\n        \"c4cpp\": \"true\"");
    }

    if (isLocal) {
        fprintf(file, ",\n        \"public\": false");
        count ++;
    }
    if (nocoverage) {
        fprintf(file, ",\n        \"coverage\": false");
    }

    fprintf(file, "\n    }");
    fprintf(file, "\n, \"corto\": { }");
    fprintf(file, "\n}\n");
    fclose(file);

    free(description);

    return 0;
error:
    return -1;
}

static char* cortotool_canonicalName(
    char *id,
    char **name,
    corto_id dir)
{
    char *id_noslash = id;

    /* Skip initial slash */
    if (id_noslash[0] == '/') {
        id_noslash ++;
    }

    /* Make a copy, so we can modify name */
    id_noslash = ut_strdup(id_noslash);

    /* Package name should start with a letter */
    if (!isalpha(id_noslash[0])) {
        ut_throw("package name '%s' does not begin with letter", id);
        goto error;
    }

    /* Validate package name */
    char *ptr, ch, *dirPtr = dir;
    for (ptr = id_noslash; (ch = *ptr); ptr ++) {
        if (!isalpha(ch) && !isdigit(ch) && ch != '/' && ch != '_' && ch != '-' && ch != '.') {
            ut_throw("invalid character '%c' in package name '%s'",
                ch, id);
            goto error;
        }
        if (ch == '-') {
            ch = *ptr = '/';
        }
        if (ch == '.') {
            ch = *ptr = '/';
        }
        if (ch == '/') {
            *dirPtr = '-';
        } else {
            *dirPtr = ch;
        }
        dirPtr ++;
    }
    *dirPtr = '\0';

    /* Get last element in package identifier */
    if (name) {
        *name = strrchr(id_noslash, '/');
        if (!*name) {
            *name = strrchr(id_noslash, '.');
        }
        if (!*name) {
            *name = id_noslash;
        } else {
            (*name) ++;
        }
    }

    return id_noslash;
error:
    return NULL;
}

static corto_int16 cortotool_app (
    const char *projectKind,
    char *projectName,
    char *dir,
    corto_bool silent,
    corto_bool mute,
    corto_bool nobuild,
    corto_bool local,
    corto_bool nocoverage,
    corto_string language,
    bool cpp)
{
    corto_id buff, dir_buffer;
    FILE *file;
    char *name = NULL;

    silent |= mute;

    char *id = cortotool_canonicalName(projectName, &name, dir_buffer);
    if (!id) {
        if (!mute) {
            ut_throw(NULL);
        }
        goto error;
    }

    if (!dir) {
        dir = dir_buffer;
    }

    if (cortotool_setupProject(projectKind, dir, id, name, local, silent)) {
        goto error;
    }

    if (cortotool_createProjectJson(
        projectKind,
        id,
        dir,
        local,
        nocoverage,
        language,
        cpp)) {
        goto error;
    }

    sprintf(buff, "%s/src", dir);
    if (ut_mkdir(buff)) {
        ut_throw("couldn't create %s directory (check permissions)", buff);
        goto error;
    }

    if (!cpp) {
        sprintf(buff, "%s/src/main.c", dir);
    } else {
        sprintf(buff, "%s/src/main.cpp", dir);
    }
    file = fopen(buff, "w");
    if (file) {
        fprintf(file, "#include <include/%s.h>\n\n", name);
        fprintf(file, "int %s(int argc, char *argv[]) {\n\n", "cortomain");
        fprintf(file, "    return 0;\n");
        fprintf(file, "}\n");
        fclose(file);
    } else {
        ut_throw("couldn't create '%s' (check permissions)", buff);
        goto error;
    }

    if (ut_chdir(dir)) {
        ut_throw("can't change working directory to '%s' (check permissions)", buff);
        goto error;
    }

    if (!nobuild) {
        int8_t ret;
        int sig;
        if ((sig = ut_proc_cmd("bake --verbosity error", &ret) || ret)) {
            ut_throw("failed to build project");
            goto error;
        }
    }

    if (!silent) {
        printf("  id = %s'%s'%s\n", UT_CYAN, id, UT_NORMAL);
        printf("  type = %s'application'%s\n", UT_CYAN, UT_NORMAL);
        printf("  language = %s'%s'%s\n", UT_CYAN , language, UT_NORMAL);
        printf("  c4cpp = %s'%s'%s\n", UT_CYAN , cpp ? "yes" : "no", UT_NORMAL);
        printf("Done! Run the app by running %s'./%s/%s'%s.\n\n", UT_CYAN, name, name, UT_NORMAL);
    }

    return 0;
error:
    return -1;
}

static corto_int16 cortotool_package(
    char *projectName,
    char *dir,
    corto_bool silent,
    corto_bool mute,
    corto_bool nobuild,
    corto_bool local,
    corto_bool nocoverage,
    corto_string language,
    bool cpp)
{
    corto_id srcfile, srcdir, dir_buffer;
    corto_char *name = NULL;

    silent |= mute;

    char *id = cortotool_canonicalName(projectName, &name, dir_buffer);
    if (!id) {
        if (!mute) {
            ut_throw(NULL);
        }
        goto error;
    }

    if (!dir) {
        dir = dir_buffer;
    }

    if (cortotool_setupProject(CORTO_PACKAGE, dir, id, name, local, silent)) {
        goto error;
    }

    if (cortotool_createProjectJson(
        CORTO_PACKAGE,
        id,
        dir,
        local,
        nocoverage,
        language,
        cpp)) {
        goto error;
    }

    /* Create src and include folders */
    sprintf(srcdir, "%s/include", dir);
    if (ut_mkdir(srcdir)) {
        ut_throw(
          "corto: failed to create directory '%s' (check permissions)",
          srcdir);
        goto error;
    }

    sprintf(srcdir, "%s/src", dir);
    if (ut_mkdir(srcdir)) {
        ut_throw(
          "corto: failed to create directory '%s' (check permissions)",
          srcdir);
        goto error;
    }

    if (!cpp) {
        snprintf(srcfile, sizeof(srcfile), "%s/src/main.c", dir);
    } else {
        snprintf(srcfile, sizeof(srcfile), "%s/src/main.cpp", dir);
    }

    /* Change working directory */
    if (ut_chdir(dir)) {
        if (!mute) {
            ut_throw(
              "corto: can't change directory to '%s' (check permissions)",
              name);
        }
        goto error;
    }

    if (!nobuild) {
        int8_t ret;
        int sig;
        if ((sig = ut_proc_cmd("bake --verbosity error", &ret) || ret)) {
            ut_throw("failed to build project");
        }
    }

    if (!silent) {
        printf("  id = %s'%s'%s\n", UT_CYAN, id, UT_NORMAL);
        printf("  type = %s'package'%s\n", UT_CYAN, UT_NORMAL);
        printf("  language = %s'%s'%s\n", UT_CYAN , language, UT_NORMAL);
        printf("  c4cpp = %s'%s'%s\n", UT_CYAN , cpp ? "yes" : "no", UT_NORMAL);
        printf("Done\n\n");
    }

    return 0;
error:
    return -1;
}

int cortomain(int argc, char *argv[]) {
    if (cortotool_main(argc, argv)) {
        goto error;
    }

    return 0;
error:
    return -1;
}

int cortotool_main(int argc, char *argv[]) {
    ut_ll silent, mute, nobuild, local;
    ut_ll apps, packages, nocoverage;
    ut_ll apps_noname, packages_noname, output, cpp;
    corto_string language = "c";
    char *outputdir = NULL;

    CORTO_UNUSED(argc);
    corto_argdata *data = corto_argparse(
      argv,
      (corto_argdata[]){
        {"$0", NULL, NULL}, /* Ignore 'create' */
        {"--silent", &silent, NULL},
        {"--mute", &mute, NULL},
        {"--nobuild", &nobuild, NULL},
        {"--local", &local, NULL},
        {"--nocoverage", &nocoverage, NULL},
        {"--use-cpp", &cpp, NULL},
        {"-o", NULL, &output},
        {CORTO_APPLICATION, NULL, &apps},
        {CORTO_PACKAGE, NULL, &packages},
        {CORTO_APPLICATION, &apps_noname, NULL},
        {CORTO_PACKAGE, &packages_noname, NULL},
        {"$1", &apps, NULL},
        {NULL}
      }
    );

    if (!data) {
        ut_throw(NULL);
        goto error;
    }

    if (output) {
        outputdir = ut_ll_get(output, 0);
    }

    /* If no arguments are provided, create an application with a random name */
    if (!apps && !packages && !apps_noname && !packages_noname)
    {
        char *name = cortotool_randomName();
        if (cortotool_app(
            CORTO_APPLICATION,
            name,
            outputdir,
            silent != NULL,
            mute != NULL,
            nobuild != NULL,
            local != NULL,
            nocoverage != NULL,
            language,
            cpp != NULL))
        {
            goto error;
        }
    }

    if (apps) {
        ut_iter iter = ut_ll_iter(apps);
        while (ut_iter_hasNext(&iter)) {
            char *name = ut_iter_next(&iter);
            if (cortotool_app(
                CORTO_APPLICATION,
                name,
                outputdir,
                silent != NULL,
                mute != NULL,
                nobuild != NULL,
                local != NULL,
                nocoverage != NULL,
                language,
                cpp != NULL))
            {
                goto error;
            }
        }
    }

    if (apps_noname) {
        ut_iter iter = ut_ll_iter(apps_noname);
        while (ut_iter_hasNext(&iter)) {
            char *name = cortotool_randomName();
            ut_iter_next(&iter);
            if (cortotool_app(
                CORTO_APPLICATION,
                name,
                outputdir,
                silent != NULL,
                mute != NULL,
                nobuild != NULL,
                local != NULL,
                nocoverage != NULL,
                language,
                cpp != NULL))
            {
                goto error;
            }
        }
    }

    if (packages) {
        ut_iter iter = ut_ll_iter(packages);
        while (ut_iter_hasNext(&iter)) {
            char *name = ut_iter_next(&iter);
            if (cortotool_package(
                name,
                outputdir,
                silent != NULL,
                mute != NULL,
                nobuild != NULL,
                local != NULL,
                nocoverage != NULL,
                language,
                cpp != NULL))
            {
                goto error;
            }
        }
    }

    if (packages_noname) {
        ut_iter iter = ut_ll_iter(packages_noname);
        while (ut_iter_hasNext(&iter)) {
            char *name = cortotool_randomName();
            ut_iter_next(&iter);
            if (cortotool_package(
                name,
                outputdir,
                silent != NULL,
                mute != NULL,
                nobuild != NULL,
                local != NULL,
                nocoverage != NULL,
                language,
                cpp != NULL))
            {
                goto error;
            }
        }
    }

    corto_argclean(data);

    return 0;
error:
    return -1;
}

void cortotool_createHelp(void) {
    printf("Usage: corto create\n");
    printf("Usage: corto create <name> [options]\n");
    printf("Usage: corto create <command> <name> [options]\n");
    printf("Usage: corto create <command> [options]\n");
    printf("\n");
    printf("When no name is passed to create, corto will choose a random name.\n");
    printf("\n");
    printf("Commands:\n");
    printf("   app:           Create a new application project. An app is a standalone\n");
    printf("                  executable that links with the corto libraries. You can\n");
    printf("                  run your application by typing 'corto run' in the app directory\n");
    printf("   package:       Create a new package. Use when you're about to create\n");
    printf("                  functionality that must be shared between apps, or\n");
    printf("                  other packages.\n");
    printf("\n");
    printf("Options:\n");
    printf("   --local        Create a project that won't be installed to the package repository\n");
    printf("   --nobuild      Do not build the project after creating it\n");
    printf("   --nocoverage   Disable coverage analysis for project\n");
    printf("   --silent       Suppress output from stdout\n");
    printf("   --mute         Suppress output from stdout and stderr\n");
    printf("\n");
}
