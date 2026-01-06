import os
import random
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import datetime
from pathlib import Path

import pytest
import requests


def test_it_proxies_responses(
    server_url,
    proxying_server_url,
    proxying_server,
    server_root: Path,
):
    for _, _, filenames in server_root.walk():
        for filename in filenames:
            print("proxying a request to", filename)
            original_response = requests.get(f"{server_url}/{filename}")
            proxied_response = requests.get(f"{proxying_server_url}/{filename}")
            assert original_response.ok and proxied_response.ok
            assert original_response.content == proxied_response.content


@pytest.mark.skipif(
    os.environ.get("LOAD_TESTING") != "1", reason="env variable LOAD_TESTING not set"
)
def test_it_proxies_a_lot(
    proxying_server,
    proxying_server_url,
    server_root,
):
    filenames = next(server_root.walk())[2]
    concurrent_requests_num = 120
    requests_num = 6000
    urls = [
        "{}/{}?seq={}".format(proxying_server_url, random.choice(filenames), idx)
        for idx in range(requests_num)
    ]
    print(
        f"Loading {len(urls)} URLs into {concurrent_requests_num} parallel requesters."
    )

    def do_work(url):
        try:
            resp = requests.get(url, timeout=1)
            return resp.ok
        except Exception as ex:
            print(
                f"{datetime.utcnow().isoformat()} requests error {url.split('?')[-1]}: {str(ex)}"
            )
            return False

    result = []
    with ThreadPoolExecutor(max_workers=concurrent_requests_num) as executor:
        futures = [executor.submit(do_work, url) for url in urls]
        print(len(executor._threads))
        for future in as_completed(futures):
            result.append(future.result())
    false_count = result.count(False)
    true_count = len(result) - false_count
    print("t->", true_count, "f->", false_count)
    threshold = int(os.environ.get("LT_PROXYING_FAILED_THRESHOLD", "0"))
    assert false_count <= threshold
