import os

from flask import Flask

from .db import db, init_app
from . import auth, operator, implant_route, download

def create_app(test_config=None):
    # create and configure app
    app = Flask(__name__, instance_relative_config=True)

    download_folder = os.path.join('static', 'test_files')

    app.config.from_mapping(
        SECRET_KEY='dev',
        SQLALCHEMY_DATABASE_URI='sqlite:///test.db',
        SQLALCHEMY_TRACK_MODIFICATIONS=False,
        DOWNLOAD_FOLDER=download_folder
    )

    # additional config
    if test_config is None:
        app.config.from_pyfile('config.py', silent=True)
    else:
        app.config.from_mapping(test_config)

    # ensure the instance folder exists
    try:
        os.makedirs(app.instance_path)
    except OSError:
        pass

    with app.app_context():
        init_app(app)  # register cli function init-db
        db.init_app(app)  # start database

    app.register_blueprint(auth.bp)
    app.register_blueprint(operator.bp)
    app.register_blueprint(implant_route.bp)
    app.register_blueprint(download.bp)
    app.add_url_rule('/', endpoint='index')

    return app
