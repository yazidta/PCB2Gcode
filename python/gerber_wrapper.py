import logging
import tempfile
from pathlib import Path
from pygerber.gerberx3.api.v2 import GerberFile, Project, FileTypeEnum
from pygerber.backend.rasterized_2d.color_scheme import ColorScheme
from pygerber.gerberx3.api._v2 import ImageFormatEnum, PixelFormatEnum
from pygerber.common.rgba import RGBA

from pygerber.gerberx3.parser2.attributes2 import ObjectAttributes, ApertureAttributes
from pygerber.gerberx3.parser2.commands2 import command2
from pygerber.gerberx3.parser2.commands2.flash2 import Flash2
from pygerber.gerberx3.parser2.state2 import State2Constants

class GerberWrapper:
    """
    Wrapper class for handling Gerber files and rendering them to images.
    """

    def __init__(self):
        logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')
        self.project = None
        logging.debug("GerberWrapper initialized.")

    def parseGerberFile(self, filePath: str, fileType: FileTypeEnum) -> GerberFile:
        """
        Parses a single Gerber file with the specified file type.
        Args:
            filePath: Path to the Gerber file.
            fileType: Type of the Gerber file (e.g., COPPER, MASK, etc.).
        Returns:
            A parsed GerberFile instance.
        """
        try:
            gerberFile = GerberFile.from_file(filePath, file_type=fileType)
            logging.debug(f"Successfully parsed Gerber file: {filePath}")
            return gerberFile
        except Exception as e:
            logging.error(f"Failed to parse Gerber file: {filePath}. Exception: {e}")
            raise

    def loadGerberFiles(self, gerberFilePaths: list):
        """
        Loads multiple Gerber files (copper, mask, silk, and edge cuts) into the project.
        Args:
            gerberFilePaths: List of file paths to the Gerber files.
        """
        try:
            if len(gerberFilePaths) == 4:
                fileTypes = [FileTypeEnum.COPPER, FileTypeEnum.MASK, FileTypeEnum.SILK, FileTypeEnum.EDGE]
            else:
                fileTypes = [FileTypeEnum.COPPER]
            self.project = Project(
                [
                    self.parseGerberFile(filePath, fileType)
                    for filePath, fileType in zip(gerberFilePaths, fileTypes)
                ]
            )
            logging.debug("Successfully initialized GerberLayerStack with all files.")
        except Exception as e:
            logging.error(f"Failed to load Gerber files. Exception: {e}")
            raise

    def clearGerberFiles(self):
        """
        Clears the currently loaded Gerber files from memory.
        """
        self.project = None
        logging.debug("Cleared all loaded Gerber files.")

    def renderToPng(self, dpmm=100):
        """
        Renders the loaded Gerber files to a PNG image, with customized colors.
        Args:
            dpmm: Dots per millimeter for rendering resolution. Defaults to 100.
        Returns:
            Path to the rendered PNG file.
        """
        if not self.project:
            logging.warning("No layer stack initialized. Please load Gerber files first.")
            return None
        try:
            with tempfile.NamedTemporaryFile(delete=False, suffix='.png') as tempFile:
                tempFilePath = tempFile.name
            logging.debug(f"Rendering to output path: {tempFilePath}")
            self.project.parse().render_raster(tempFilePath, dpmm=60)
            logging.debug(f"Successfully rendered GerberLayerStack to PNG: {tempFilePath}")
            return tempFilePath
        except Exception as e:
            logging.error(f"Failed to render Gerber layer stack. Exception: {e}")
            raise

    def processBoundingBox(self, boundingBox):
        try:
            minX = boundingBox['minX']
            minY = boundingBox['minY']
            maxX = boundingBox['maxX']
            maxY = boundingBox['maxY']
            width = maxX - minX
            height = maxY - minY
            logging.debug(f"Bounding box width: {width}, height {height}")
        except KeyError as e:
            logging.error(f"Missing key in bounding box: {e}")

    def getBoundingBox(self):
        if not self.project:
            logging.warning("No Gerber files loaded to calculate the bounding box.")
            return None

        try:
            allMinX = []
            allMinY = []
            allMaxX = []
            allMaxY = []

            for gerberFile in self.project.files:
                parsedFile = gerberFile.parse()
                fileInfo = parsedFile.get_info()

                allMinX.append(float(fileInfo.min_x_mm))
                allMinY.append(float(fileInfo.min_y_mm))
                allMaxX.append(float(fileInfo.max_x_mm))
                allMaxY.append(float(fileInfo.max_y_mm))

            minX = min(allMinX)
            minY = min(allMinY)
            maxX = max(allMaxX)
            maxY = max(allMaxY)

            boundingBoxDict = {
                "minX": minX,
                "minY": minY,
                "maxX": maxX,
                "maxY": maxY,
            }

            self.processBoundingBox(boundingBoxDict)
            logging.debug(f"Bounding box calculated: {boundingBoxDict}")
            return boundingBoxDict

        except Exception as e:
            logging.error(f"Failed to fetch bounding box. Exception: {e}")
            raise
    def extractPadCoords(self):
        """
        Extracts pad coordinates from the copper layer Gerber file.

        Returns:
            A list of dictionaries containing pad coordinates.
        """
        if not self.project:
            logging.warning("No Gerber files loaded to extract pad coordinates.")
            return []

        try:
            padCoords = []
            copperFile = self.project.files[0]
            parsedFile = copperFile.parse()
            #logging.debug(f"Attribute of parsedFile: {dir(parsedFile)})

            for command in parsedFile._command_buffer.commands:
                if isinstance(command, Flash2):
                    ##logging.debug(f"Type of flash_point.x: {type(command.flash_point.x)}")
                    ##logging.debug(f"flash_point.x attributes: {dir(command.flash_point.x)}")
                    flashX = command.flash_point.x
                    flashY = command.flash_point.y

                    # Debugging: Print type and attributes
                    logging.debug(f"flashX: {flashX}, flashY: {flashY}")
                    logging.debug(f"Type of flashX: {type(flashX)}, Type of flashY: {type(flashY)}")
                    try:
                        x = float(flashX.value)
                        y = float(flashY.value)
                    except AttributeError as ae:
                        logging.error(f"Attribute error while extracting float from Offset: {ae}")
                        continue
                    except TypeError as te:
                        logging.error(f"Type error while converting Offset to float: {te}")
                        continue
                    except Exception as e:
                        logging.error(f"Unexpected error while extracting float from Offset: {e}")
                        continue

                    aperture = command.aperture
                    logging.debug(f"aperture: {aperture}")
                    logging.debug(f"Type of aperture: {type(aperture)}")
                    logging.debug(f"Aperture attributes: {dir(aperture)}")

                    if hasattr(aperture, 'identifier'):
                        apertureCode = aperture.identifier
                        logging.debug(f"Retrieved aperture code via 'identifier': {apertureCode}")
                    else:
                        apertureCode = "Unknown"
                        logging.debug("Aperture code could not be retrieved. Set to 'Unknown'.")
                    padInfo = {
                        "x": x,
                        "y": y,
                        "aperture": apertureCode
                    }
                    padCoords.append(padInfo)
                    logging.debug(f"Pad found at X: {x}, Y: {y}, Aperture: {apertureCode}")

            logging.debug(f"Extracted {len(padCoords)} pad coordinates.")
            return padCoords

        except Exception as e:
            logging.error(f"Failed to extract pad coordinates. Exception: {e}")
            raise

