import requests


def test_simple_not_found(server_url):
    resp = requests.get("{}/utter-nonsense-yes".format(server_url))
    assert resp.status_code == 404


def test_it_returns_index_html_by_default(server_url):
    resp = requests.get(server_url + "/")
    assert resp.status_code == 200
    assert "Hello, world!" in resp.text
    assert resp.headers["Content-Type"] == "text/html"


def test_it_returns_files(server_url):
    resp = requests.get(server_url + "/image.jpg")
    assert resp.status_code == 200
    assert resp.headers["Content-Type"] == "image/jpg"

    resp = requests.get(server_url + "/chords.pdf")
    assert resp.status_code == 200
    assert resp.headers["Content-Type"] == "application/pdf"
