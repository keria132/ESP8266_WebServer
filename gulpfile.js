//Gulp plugins
const gulp = require('gulp');
const browserSync = require("browser-sync").create();
const sass = require('gulp-sass')(require('sass'));
const cleanCss = require("gulp-clean-css");
const autoprefixer = require('gulp-autoprefixer');
const rename = require("gulp-rename");
const sourcemaps = require("gulp-sourcemaps");
const uglify = require("gulp-uglify");

//Browsersync settings

function sync(cd){

	browserSync.init({

		server: {
			baseDir: './'
		},
		port: 1030
    })
    
	cd();
}

//SASS compile function
function sassCompiler(cd){
    gulp.src('./src/sass/**/*.scss')
    .pipe(sourcemaps.init())
    .pipe(sass().on('error', sass.logError))
    .pipe(cleanCss({compatibility: 'ie8'}))
    .on('error', console.error.bind(console))
    .pipe(autoprefixer(
        ['> 1%',
        'last 2 versions',
        'firefox >= 4',
        'safari 7',
        'safari 8',
        'IE 8',
        'IE 9',
        'IE 10',
        'IE 11'],
    { cascade: false }))
    .pipe(rename({suffix: '.min'}) )
    .pipe(sourcemaps.write('.') )
    .pipe(gulp.dest('./src/css/') )
    .pipe(browserSync.stream() );

    cd();
}

//Javascript compile function
function javascript(cd){
    gulp.src("./src/js/*.js")
    .pipe(sourcemaps.init())
    .pipe(uglify())
    .pipe(rename({suffix: '.min'}) )
    .pipe(sourcemaps.write('.'))
    .pipe(gulp.dest("./src/js/min/"))
    .pipe(browserSync.stream())

    cd();
}

//Watch for files changes
function watchFiles(){
	gulp.watch('./*.html', function browserReload(done) {
        browserSync.reload();
        done();
    });
	gulp.watch('src/js/*.js', javascript);
    gulp.watch('src/sass/*.scss', sassCompiler);
}

//Build production version

function build(cb){

    gulp.src('./src/sass/**/*.scss')
		.pipe(sourcemaps.init())
		.pipe(sass().on('error', sass.logError))
		.pipe(cleanCss({compatibility: 'ie8'}))
		.on('error', console.error.bind(console))
		.pipe(autoprefixer(
						['> 1%',
                        'last 2 versions',
                        'firefox >= 4',
                        'safari 7',
                        'safari 8',
                        'IE 8',
                        'IE 9',
                        'IE 10',
                        'IE 11'],
			{ cascade: false }))
		.pipe(rename({suffix: '.min'}))
		.pipe(sourcemaps.write('./') )
		.pipe(gulp.dest('./build/css/'));

	gulp.src('./src/js/script.js')
        .pipe(sourcemaps.init())
        .pipe(uglify())
        .pipe(rename({suffix: '.min'}))
        .pipe(sourcemaps.write('.'))
		.pipe(gulp.dest('./build/js/'));

	gulp.src('./src/assets/*.*')
		.pipe( gulp.dest('./build/assets'));
		
	gulp.src('./src/index.html')
			.pipe( gulp.dest('./build/') );
	
	cb();
}

//Create project structure

function dirs(cb){

    gulp.src("*.*", {read: false})
    .pipe(gulp.dest("./build/"))
    .pipe(gulp.dest("./src/css/"))
    .pipe(gulp.dest("./src/sass/"))
    .pipe(gulp.dest("./src/js/"))
    .pipe(gulp.dest("./src/assets/"))
    .pipe(gulp.dest("./src/libs/"));

    cb();
}

exports.default = gulp.parallel(sync, watchFiles);
exports.sass = sassCompiler;
exports.js = javascript;
exports.build = build;
exports.dirs = dirs;
// exports.default = () => {console.log("well well well...")}