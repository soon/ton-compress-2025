import asyncio
import aiohttp
import re
import os
import random

async def fetch_url(seqno):
    url = f"https://explorer.toncenter.com/search?workchain=-1&shard=8000000000000000&seqno={seqno}"
    base_url = "https://explorer.toncenter.com"

    file_path = f'blocks/seqno={seqno}.boc'
    if os.path.exists(file_path):
        return

    async with aiohttp.ClientSession() as session:
        try:
            async with session.get(url) as response:
                if response.status == 200:
                    html_content = await response.text()

                    # Use regex to find hrefs starting with /download
                    download_links = re.findall(r'href="(/download[^"]*)"', html_content)

                    if download_links:
                        for link in download_links:
                            full_url = base_url + link
                            async with session.get(full_url) as file_response:
                                if file_response.status == 200:
                                    with open(file_path, 'wb') as f:
                                        f.write(await file_response.read())
                                    print(f"File saved as {file_path}")
                                else:
                                    print(f"Failed to download {full_url}, status code: {file_response.status}")
                    else:
                        print("No download links found.")
                else:
                    print(f"Failed to fetch URL, status code: {response.status}")
        except Exception as e:
            print(f"An error occurred: {e}")

async def main():
    seqnos = range(43888281 - 100, 43888281)
    tasks = []
    semaphore = asyncio.Semaphore(10)  # Limit to 10 concurrent tasks

    async def semaphore_task(seqno):
        async with semaphore:
            await fetch_url(seqno)

    for seqno in seqnos:
        tasks.append(semaphore_task(seqno))

    await asyncio.gather(*tasks)

if __name__ == "__main__":
    asyncio.run(main())