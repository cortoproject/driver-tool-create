
#include <driver/tool/create/create.h>
#include <corto/argparse/argparse.h>

#define CORTO_PACKAGE ("package")
#define CORTO_APPLICATION ("application")

#define CORTO_PROMPT CORTO_CYAN "corto: " CORTO_NORMAL

int cortomain(int argc, char *argv[]);

static corto_int16 cortotool_setupProject(
    const char *projectKind,
    const char *dir,
    corto_bool isLocal,
    corto_bool isSilent)
{
    CORTO_UNUSED(isLocal);
    CORTO_UNUSED(projectKind);

    if (corto_file_test(dir)) {
        corto_id id;
        sprintf(id, "%s/.corto", dir);
        if (corto_file_test(id)) {
            corto_throw(
                "corto: a project in location '%s' already exists!",
                dir);
            goto error;
        }
    } else if (!corto_mkdir(dir)) {
        corto_id id;
        sprintf(id, "%s/.corto", dir);
        if (corto_mkdir(id)) {
            corto_throw(
                "corto: couldn't create '%s/.corto (check permissions)'",
                dir);
            goto error;
        }

    } else {
        corto_throw(
            "corto: couldn't create project directory '%s' (check permissions)",
            dir);
        goto error;
    }

    if (!isSilent) {
        printf("\n");
        printf("               \\ /\n");
        printf("              - . -\n");
        printf("               /%s|%s\\\n", CORTO_GREEN, CORTO_NORMAL);
        printf("%s                | /\\\n", CORTO_GREEN);
        printf("%s             /\\ |/\n", CORTO_GREEN);
        printf("%s               \\|%s\n", CORTO_GREEN, CORTO_NORMAL);
        printf ("\nPlanting a new idea in directory %s'%s'%s\n",
            CORTO_CYAN,
            dir,
            CORTO_NORMAL);
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
    corto_bool nocorto,
    corto_bool nocoverage,
    corto_string language)
{
    FILE *file;
    corto_id buff;
    int8_t count = 0;

    sprintf(buff, "%s/project.json", dir);
    file = fopen(buff, "w");
    if(!file) {
        corto_throw("couldn't create %s/project.json (check permissions)", buff);
        goto error;
    }

    fprintf(file,
        "{\n"\
        "    \"id\": \"%s\",\n"\
        "    \"type\": \"%s\",\n"\
        "    \"value\": {",
        id,
        !strcmp(projectKind, CORTO_PACKAGE) ? "package" : "application");

    fprintf(file,  "\n        \"description\": \"Making the world a better place\"");
    fprintf(file, ",\n        \"author\": \"Arthur Dent\"");
    fprintf(file, ",\n        \"version\": \"1.0.0\"");
    fprintf(file, ",\n        \"language\": \"%s\"", language);

    if (isLocal) {
        fprintf(file, ",\n        \"public\": false");
        count ++;
    }
    if (nocorto) {
        fprintf(file, ",\n        \"managed\": false");
    }
    if (nocoverage) {
        fprintf(file, ",\n        \"coverage\": false");
    }

    fprintf(file, "\n    }\n}\n");
    fclose(file);

    return 0;
error:
    return -1;
}

static corto_int16 cortotool_createTest(corto_string id, corto_bool isPackage, corto_bool isLocal, corto_string language) {
    FILE *file;

    if (corto_mkdir("test")) {
        corto_throw("couldn't create test directory for '%s' (check permissions)", id);
        goto error;
    }
    if (corto_mkdir("test/src")) {
        corto_throw("couldn't create test/src directory for '%s' (check permissions)", id);
        goto error;
    }

    if (cortomain(
        8,
        (char*[]){
            "create",
            "package",
            "/test",
            "--notest",
            "--local",
            "--silent",
            "--nobuild",
            "--nocoverage",
            "--lang",
            language,
            NULL}
    )) {
        corto_throw("couldn't create test skeleton (check permissions)");
        goto error;
    }

    corto_id filename;
    if (!strcmp(language, "c")) {
        sprintf(filename, "src/test.c");
    } else {
        sprintf(filename, "src/test.cpp");
    }
    file = fopen(filename, "w");
    if (file) {
        fprintf(file, "#include <include/test.h>\n");
        fprintf(file, "\n");
        fprintf(file, "int cortomain(int argc, char *argv[]) {\n");
        fprintf(file, "    int result = 0;\n");
        fprintf(file, "    test_Runner runner = test_RunnerCreate(\"%s\", argv[0], (argc > 1) ? argv[1] : NULL);\n", id);
        fprintf(file, "    if (!runner) return -1;\n");
        fprintf(file, "    if (corto_ll_count(runner->failures)) {\n");
        fprintf(file, "        result = -1;\n");
        fprintf(file, "    }\n");
        fprintf(file, "    corto_delete(runner);\n");
        fprintf(file, "    return result;\n");
        fprintf(file, "}\n");
        fclose(file);
    } else {
        corto_throw("couldn't create 'test/%s' (check permissions)", filename);
        goto error;
    }

    file = fopen("model.cx", "w");
    if (file) {
        fprintf(file, "in package test\n\n");
        fprintf(file, "test/Suite MySuite:/\n");
        fprintf(file, "    void testSomething()\n\n");
        fclose(file);
    } else {
        corto_throw("couldn't create 'test/model.cx' (check permissions)");
        goto error;
    }

    if (corto_run(
        "driver/tool/add",
        2,
        (char*[]){"add", "/corto/test", NULL}))
    {
        corto_throw("failed to add corto/test package");
        goto error;
    }

    if (isPackage && !isLocal) {
        if (corto_run(
            "driver/tool/add",
            2,
            (char*[]){"add", id, NULL}))
        {
            corto_throw("failed to add '%s' package", id);
            goto error;
        }
    }

    /* Timestamps of the files are likely too close to trigger a build, so explicitly
     * do a rebuild of the test project */
     int8_t ret;
     int sig;
     if ((sig = corto_proc_cmd("bake rebuild --error", &ret) || ret)) {
         corto_throw("failed to rebuild test");
     }

    if (corto_chdir("..")) {
        corto_throw("failed to change directory to parent");
        goto error;
    }

    return 0;
error:
    return -1;
}

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
    id_noslash = corto_strdup(id_noslash);

    /* Package name should start with a letter */
    if (!isalpha(id_noslash[0])) {
        corto_throw("package name '%s' does not begin with letter", id);
        goto error;
    }

    /* Validate package name */
    char *ptr, ch, *dirPtr = dir;
    for (ptr = id_noslash; (ch = *ptr); ptr ++) {
        if (!isalpha(ch) && !isdigit(ch) && ch != '/' && ch != '_' && ch != '-') {
            corto_throw("invalid character '%c' in package name '%s'",
                ch, id);
            goto error;
        }
        if (ch == '-') {
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
    corto_bool notest,
    corto_bool local,
    corto_bool nocorto,
    corto_bool nocoverage,
    corto_string language)
{
    corto_id buff, dir_buffer;
    FILE *file;
    char *name = NULL;

    silent |= mute;

    char *id = cortotool_canonicalName(projectName, &name, dir_buffer);
    if (!id) {
        if (!mute) {
            corto_throw(NULL);
        }
        goto error;
    }

    if (!dir) {
        dir = dir_buffer;
    }

    if (cortotool_setupProject(projectKind, dir, local, silent)) {
        goto error;
    }

    if (cortotool_createProjectJson(projectKind, id, dir, local, nocorto, nocoverage, language)) {
        goto error;
    }

    sprintf(buff, "%s/src", dir);
    if (corto_mkdir(buff)) {
        corto_throw("couldn't create %s directory (check permissions)", buff);
        goto error;
    }

    if (!strcmp(language, "c")) {
        sprintf(buff, "%s/src/%s.c", dir, name);
    } else {
        sprintf(buff, "%s/src/%s.cpp", dir, name);
    }
    file = fopen(buff, "w");
    if (file) {
        if (!nocorto) {
            fprintf(file, "#include <include/%s.h>\n\n", name);
        }
        fprintf(file, "int %s(int argc, char *argv[]) {\n\n", nocorto ? "main" : "cortomain");
        fprintf(file, "    return 0;\n");
        fprintf(file, "}\n");
        fclose(file);
    } else {
        corto_throw("couldn't create '%s' (check permissions)", buff);
        goto error;
    }

    if (corto_chdir(dir)) {
        corto_throw("can't change working directory to '%s' (check permissions)", buff);
        goto error;
    }

    if (!nobuild) {
        int8_t ret;
        int sig;
        if ((sig = corto_proc_cmd("bake --error", &ret) || ret)) {
            corto_throw("failed to build project");
        }
    }

    if (!notest) {
        if (cortotool_createTest(
            id,
            FALSE,
            local,
            language))
        {
            goto error;
        }
    }

    if (!silent) {
        printf("  id = %s'%s'%s\n", CORTO_CYAN, id, CORTO_NORMAL);
        printf("  type = %s'application'%s\n", CORTO_CYAN, CORTO_NORMAL);
        printf("  language = %s'%s'%s\n", CORTO_CYAN , language, CORTO_NORMAL);
        printf("  managed = %s'%s'%s\n", CORTO_CYAN , nocorto ? "no" : "yes", CORTO_NORMAL);
        printf("Done! Run the app by running %s'./%s/%s'%s.\n\n", CORTO_CYAN, name, name, CORTO_NORMAL);
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
    corto_bool notest,
    corto_bool local,
    corto_bool nocorto,
    corto_bool nodef,
    corto_bool nocoverage,
    corto_string language,
    bool cpp)
{
    corto_id cxfile, srcfile, srcdir, dir_buffer;
    corto_char *name = NULL;
    FILE *file;

    silent |= mute;

    char *id = cortotool_canonicalName(projectName, &name, dir_buffer);
    if (!id) {
        if (!mute) {
            corto_throw(NULL);
        }
        goto error;
    }

    if (!dir) {
        dir = dir_buffer;
    }

    if (cortotool_setupProject(CORTO_PACKAGE, dir, local, silent)) {
        goto error;
    }

    if (cortotool_createProjectJson(CORTO_PACKAGE, id, dir, local, nocorto, nocoverage, language)) {
        goto error;
    }

    /* Write definition file */
    if (!nocorto) {
        if (snprintf(cxfile, sizeof(cxfile), "%s/model.cx", dir) >=
            (int)sizeof(cxfile))
        {
            if (!mute) {
                corto_throw("package name '%s' is too long", id);
            }
            goto error;
        }

        if (corto_file_test(cxfile)) {
            if (!mute) {
                corto_throw("package '%s' already exists", cxfile);
            }
            goto error;
        }
    }

    if (corto_mkdir(dir)) {
        if (!mute) {
            corto_throw(
                "corto: failed to create directory '%s' (check permissions)",
                name);
        }
        goto error;
    }

    if (!nodef && !nocorto) {
        file = fopen(cxfile, "w");
        if (file) {
            fprintf(file, "in package /%s\n\n", id);
            fclose(file);
        } else {
            corto_throw(
                "corto: failed to open file '%s' (check permissions)",
                cxfile);
            goto error;
        }
    }

    /* Create src and include folders */
    sprintf(srcdir, "%s/include", dir);
    if (corto_mkdir(srcdir)) {
        corto_throw(
          "corto: failed to create directory '%s' (check permissions)",
          srcdir);
        goto error;
    }

    sprintf(srcdir, "%s/src", dir);
    if (corto_mkdir(srcdir)) {
        corto_throw(
          "corto: failed to create directory '%s' (check permissions)",
          srcdir);
        goto error;
    }

    /* When package doesn't have a definition, create an empty header and source
     * file upon creation of the package. The header is mandatory- at least one
     * header with the name of the package must exist. These files will be
     * untouched by code generation when rebuilding the package with nocorto */
    if (nocorto || nodef) {
        if (snprintf(srcfile,
            sizeof(srcfile),
            "%s/include/%s.h",
            dir, name) >= (int)sizeof(srcfile))
        {
            if (!mute) {
                corto_throw("package name '%s' is too long", name);
            }
            goto error;
        }

        /* Don't overwrite file if it already exists */
        if (!corto_file_test(srcfile)) {
            file = fopen(srcfile, "w");
            if (file) {
                /* Create macro identifier the hard way */
                corto_id macro;
                char *ptr = macro, ch;
                strcpy(macro, id);
                strupper(macro);
                while ((ch = *ptr)) {
                    if (ch == '/') {
                        *ptr = '_';
                    } else if (ch == ':') {
                        *ptr = '_';
                        memmove(ptr, ptr + 1, strlen(ptr + 1));
                    }
                    ptr ++;
                }

                fprintf(file, "\n");
                fprintf(file, "#ifndef %s_H\n", macro);
                fprintf(file, "#define %s_H\n", macro);
                fprintf(file, "\n");
                fprintf(file, "/* Add include files here */\n");
                fprintf(file, "\n");
                fprintf(file, "#ifdef __cplusplus\n");
                fprintf(file, "extern \"C\" {\n");
                fprintf(file, "#endif\n");
                fprintf(file, "\n");
                fprintf(file, "/* Insert definitions here */\n");
                fprintf(file, "\n");
                fprintf(file, "#ifdef __cplusplus\n");
                fprintf(file, "}\n");
                fprintf(file, "#endif\n\n");
                fprintf(file, "#endif /* %s_H */\n\n", macro);
                fclose(file);
            } else {
                if (!mute) {
                    corto_throw("failed to open file '%s'", srcfile);
                }
                goto error;
            }
        }
    }

    if (!strcmp(language, "c")) {
        snprintf(srcfile, sizeof(srcfile), "%s/src/%s.c", dir, name);
    } else {
        snprintf(srcfile, sizeof(srcfile), "%s/src/%s.cpp", dir, name);
    }

    /* Create main function for unmanaged packages */
    if (nocorto) {
        /* Don't overwrite file if it already exists */
        if (!corto_file_test(srcfile)) {
            file = fopen(srcfile, "w");
            if (file) {
                fprintf(file, "\n");
                if (local) {
                    fprintf(file, "#include <include/%s.h>\n", name);
                } else {
                    fprintf(file, "#include <%s/%s.h>\n", id, name);
                }
                fprintf(file, "\n");
                if (cpp) {
                    fprintf(file, "extern \"C\"\n");
                }
                fprintf(file, "int cortomain(int argc, char *argv[]) {\n\n");
                fprintf(file, "    return 0;\n");
                fprintf(file, "}\n");
                fprintf(file, "\n");
                fclose(file);
            } else {
                if (!mute) {
                    corto_throw("failed to open file '%s'", srcfile);
                }
                goto error;
            }
        }
    }

    /* Change working directory */
    if (corto_chdir(dir)) {
        if (!mute) {
            corto_throw(
              "corto: can't change directory to '%s' (check permissions)",
              name);
        }
        goto error;
    }

    if (!nobuild) {
        int8_t ret;
        int sig;
        if ((sig = corto_proc_cmd("bake --error", &ret) || ret)) {
            corto_throw("failed to build project");
        }
    }

    if (!notest) {
        if (cortotool_createTest(id, TRUE, local, language)) {
            goto error;
        }
    }

    if (!silent) {
        printf("  id = %s'%s'%s\n", CORTO_CYAN, id, CORTO_NORMAL);
        printf("  type = %s'package'%s\n", CORTO_CYAN, CORTO_NORMAL);
        printf("  language = %s'%s'%s\n", CORTO_CYAN , language, CORTO_NORMAL);
        printf("  managed = %s'%s'%s\n", CORTO_CYAN , nocorto ? "no" : "yes", CORTO_NORMAL);
        printf("Done\n\n");
    }

    return 0;
error:
    return -1;
}

int cortomain(int argc, char *argv[]) {
    corto_ll silent, mute, nobuild, notest, local;
    corto_ll apps, packages, nocorto, nodef, nocoverage;
    corto_ll apps_noname, packages_noname, lang, output, cpp;
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
        {"--notest", &notest, NULL},
        {"--unmanaged", &nocorto, NULL},
        {"--nodef", &nodef, NULL},
        {"--local", &local, NULL},
        {"--nocoverage", &nocoverage, NULL},
        {"--lang", NULL, &lang},
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
        corto_throw(NULL);
        goto error;
    }

    if (nocorto) {
        notest = nocorto;
    }

    if (lang) {
        language = corto_ll_get(lang, 0);
    }

    if (output) {
        outputdir = corto_ll_get(output, 0);
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
            notest != NULL,
            local != NULL,
            nocorto != NULL,
            nocoverage != NULL,
            language))
        {
            goto error;
        }
    }

    if (apps) {
        corto_iter iter = corto_ll_iter(apps);
        while (corto_iter_hasNext(&iter)) {
            char *name = corto_iter_next(&iter);
            if (cortotool_app(
                CORTO_APPLICATION,
                name,
                outputdir,
                silent != NULL,
                mute != NULL,
                nobuild != NULL,
                notest != NULL,
                local != NULL,
                nocorto != NULL,
                nocoverage != NULL,
                language))
            {
                goto error;
            }
        }
    }

    if (apps_noname) {
        corto_iter iter = corto_ll_iter(apps_noname);
        while (corto_iter_hasNext(&iter)) {
            char *name = cortotool_randomName();
            corto_iter_next(&iter);
            if (cortotool_app(
                CORTO_APPLICATION,
                name,
                outputdir,
                silent != NULL,
                mute != NULL,
                nobuild != NULL,
                notest != NULL,
                local != NULL,
                nocorto != NULL,
                nocoverage != NULL,
                language))
            {
                goto error;
            }
        }
    }

    if (packages) {
        corto_iter iter = corto_ll_iter(packages);
        while (corto_iter_hasNext(&iter)) {
            char *name = corto_iter_next(&iter);
            if (cortotool_package(
                name,
                outputdir,
                silent != NULL,
                mute != NULL,
                nobuild != NULL,
                notest != NULL,
                local != NULL,
                nocorto != NULL,
                nodef != NULL,
                nocoverage != NULL,
                language,
                cpp != NULL))
            {
                goto error;
            }
        }
    }

    if (packages_noname) {
        corto_iter iter = corto_ll_iter(packages_noname);
        while (corto_iter_hasNext(&iter)) {
            char *name = cortotool_randomName();
            corto_iter_next(&iter);
            if (cortotool_package(
                name,
                outputdir,
                silent != NULL,
                mute != NULL,
                nobuild != NULL,
                notest != NULL,
                local != NULL,
                nocorto != NULL,
                nodef != NULL,
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
    printf("   --unmanaged    Create an unmanaged project that doens't use corto code generation\n");
    printf("   --notest       Do not create a test skeleton\n");
    printf("   --nodef        Do not generate a definition file\n");
    printf("   --nobuild      Do not build the project after creating it\n");
    printf("   --nocoverage   Disable coverage analysis for project\n");
    printf("   --silent       Suppress output from stdout\n");
    printf("   --mute         Suppress output from stdout and stderr\n");
    printf("\n");
}
