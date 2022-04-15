from flask import (
    Blueprint, current_app, jsonify, make_response, send_from_directory
)
import os

bp = Blueprint('download', __name__)


# TODO: Sanitize filename
@bp.route('/download/<path:filename>', methods=["GET"])
def download(filename):
    directory = os.path.join(current_app.root_path, current_app.config['DOWNLOAD_FOLDER'])
    print(f"directory {directory}")

    if is_directory_traversal(directory, filename):
        print("directory traversal exists")
        response = {"error": "Cannot access file or file not found"}
        return make_response(jsonify(response), 400)

    else:
        return send_from_directory(directory, filename, as_attachment=True)


def is_directory_traversal(directory, file_name):
    requested_path = os.path.join(directory, file_name)
    requested_path = os.path.abspath(requested_path)
    common_prefix = os.path.commonprefix([requested_path, directory])
    return common_prefix != directory
