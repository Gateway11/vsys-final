#!/usr/bin/python3
"""
WAV Frame Extractor
A tool for analyzing WAV files, inspecting timestamps, and extracting audio segments
Usage: wav_frame_extractor <wav_file> [output_directory] [chunk_size]
"""

import struct
import os
import sys
import shutil

def parse_wav_header(filename, chunk_size=128):
    """
    Parse WAV file header and return frame related information
    """
    try:
        with open(filename, 'rb') as f:
            # Read RIFF header
            chunk_id = f.read(4)
            if chunk_id != b'RIFF':
                raise ValueError("Not a valid WAV file")
            
            chunk_size_total = struct.unpack('<I', f.read(4))[0]
            format = f.read(4)
            
            # Read fmt subchunk
            subchunk1_id = f.read(4)
            if subchunk1_id != b'fmt ':
                raise ValueError("fmt subchunk not found")
            
            subchunk1_size = struct.unpack('<I', f.read(4))[0]
            audio_format = struct.unpack('<H', f.read(2))[0]
            num_channels = struct.unpack('<H', f.read(2))[0]
            sample_rate = struct.unpack('<I', f.read(4))[0]
            byte_rate = struct.unpack('<I', f.read(4))[0]
            block_align = struct.unpack('<H', f.read(2))[0]
            bits_per_sample = struct.unpack('<H', f.read(2))[0]
            
            # Find data subchunk
            while True:
                subchunk2_id = f.read(4)
                if not subchunk2_id:
                    raise ValueError("data subchunk not found")
                
                if subchunk2_id == b'data':
                    break
                
                # Skip other subchunks
                subchunk2_size = struct.unpack('<I', f.read(4))[0]
                f.seek(subchunk2_size, 1)
            
            # Read data subchunk size
            subchunk2_size = struct.unpack('<I', f.read(4))[0]
            data_start_pos = f.tell()  # Record audio data start position
            
            # Calculate frame count considering chunk_size
            bytes_per_sample = bits_per_sample // 8
            bytes_per_frame = num_channels * bytes_per_sample
            bytes_per_timestamp_frame = chunk_size * bytes_per_frame
            
            # Total frames based on data size and chunk organization
            total_timestamp_frames = subchunk2_size // bytes_per_timestamp_frame
            remaining_bytes = subchunk2_size % bytes_per_timestamp_frame
            remaining_frames = remaining_bytes // bytes_per_frame
            
            total_audio_frames = (total_timestamp_frames * chunk_size) + remaining_frames
            duration = total_audio_frames / sample_rate
            
            print(f"File: {filename}")
            #print(f"Audio frames: {total_audio_frames}")
            print(f"Timestamp frames: {total_audio_frames // chunk_size}")
            print(f"Sample rate: {sample_rate} Hz")
            print(f"Channels: {num_channels}")
            print(f"Bit depth: {bits_per_sample} bits")
            print(f"Duration: {duration:.2f} seconds")
            print(f"Data size: {subchunk2_size} bytes")
            
            return {
                'audio_frames': total_audio_frames,
                'sample_rate': sample_rate,
                'channels': num_channels,
                'bits_per_sample': bits_per_sample,
                'data_start_pos': data_start_pos,
                'data_size': subchunk2_size,
                'duration': duration,
                'bytes_per_sample': bytes_per_sample,
                'block_align': block_align,
                'chunk_size': chunk_size,
                'bytes_per_frame': bytes_per_frame,
                'bytes_per_timestamp_frame': bytes_per_timestamp_frame,
                'total_timestamp_frames': total_timestamp_frames,
                'remaining_frames': remaining_frames
            }
            
    except Exception as e:
        print(f"Error: {e}")
        return None

def load_timestamps(ts_filename):
    """
    Load timestamp file, one timestamp per line
    """
    if not os.path.exists(ts_filename):
        print(f"Timestamp file {ts_filename} does not exist")
        return None
    
    timestamps = []
    try:
        with open(ts_filename, 'r') as f:
            for line_num, line in enumerate(f, 1):
                line = line.strip()
                if line:  # Skip empty lines
                    try:
                        timestamp = int(line)
                        timestamps.append(timestamp)
                    except ValueError:
                        print(f"Warning: Line {line_num} is not a valid timestamp: {line}")
        
        print(f"Loaded {len(timestamps)} timestamps from {ts_filename}")
        return timestamps
        
    except Exception as e:
        print(f"Error reading timestamp file: {e}")
        return None

def save_wav_segment(input_file, output_file, wav_info, start_frame, end_frame=None):
    """
    Save a segment of the WAV file from start_frame to end_frame
    If end_frame is None, save from start_frame to the end
    """

    start_timestamp = start_frame // wav_info['chunk_size']
    end_timestamp = end_frame // wav_info['chunk_size']
    try:
        if end_frame is None:
            end_frame = wav_info['audio_frames'] - 1
        
        if start_frame < 0 or start_frame >= wav_info['audio_frames']:
            #print(f"Error: Start frame {start_frame} is out of range")
            print(f"Error: Start timestamp frame {start_timestamp} is out of range")
            return False
        
        if end_frame < start_frame or end_frame >= wav_info['audio_frames']:
            #print(f"Error: End frame {end_frame} is out of range")
            print(f"Error: End timestamp frame {end_timestamp} is out of range")
            return False
        
        # Calculate byte positions
        bytes_per_frame = wav_info['bytes_per_frame']
        start_byte = wav_info['data_start_pos'] + (start_frame * bytes_per_frame)
        end_byte = wav_info['data_start_pos'] + ((end_frame + 1) * bytes_per_frame)
        segment_size = end_byte - start_byte
        
        #print(f"Extracting audio frames {start_frame} to {end_frame}")
        print(f"Extracting timestamp frames {start_timestamp} to {end_timestamp}")
        
        # Read the original header
        with open(input_file, 'rb') as infile:
            header_data = infile.read(wav_info['data_start_pos'])
            
            # Update the data size in the header
            header_bytearray = bytearray(header_data)
            
            # Find the position of the data chunk size field
            data_marker_pos = header_data.find(b'data')
            if data_marker_pos != -1:
                # Update the data size (little-endian)
                data_size_pos = data_marker_pos + 4
                header_bytearray[data_size_pos:data_size_pos+4] = struct.pack('<I', segment_size)
                
                # Update the overall file size (at position 4)
                file_size = len(header_bytearray) + segment_size - 8  # Subtract 8 for "RIFF" and size field
                header_bytearray[4:8] = struct.pack('<I', file_size)
            
            # Write the updated header to the output file
            with open(output_file, 'wb') as outfile:
                outfile.write(header_bytearray)
                
                # Seek to the start position in the input file
                infile.seek(start_byte)
                
                # Copy the audio data segment
                bytes_remaining = segment_size
                while bytes_remaining > 0:
                    read_size = min(8192, bytes_remaining)
                    data = infile.read(read_size)
                    if not data:
                        break
                    outfile.write(data)
                    bytes_remaining -= len(data)
        
        print(f"Successfully saved segment to: {output_file}")
        print(f"Segment duration: {(end_frame - start_frame + 1) / wav_info['sample_rate']:.2f} seconds")
        return True
        
    except Exception as e:
        print(f"Error saving WAV segment: {e}")
        return False

def select_frame_and_extract(wav_info, timestamps, input_file, output_dir):
    """
    Interactive frame selection and extract audio segments
    """
    total_audio_frames = wav_info['audio_frames']
    sample_rate = wav_info['sample_rate']
    chunk_size = wav_info['chunk_size']
    total_timestamp_frames = wav_info['total_timestamp_frames']
    
    # Calculate number of timestamp frames
    timestamp_frames = len(timestamps) if timestamps else 0
    
    print(f"\n=== Frame Selection and Extraction Mode ===")
    #print(f"Audio frames: {total_audio_frames}")
    print(f"Timestamp frames: {timestamp_frames}")
    print(f"Duration: {wav_info['duration']:.2f} seconds")
    print(f"Output directory: {output_dir}")
    print("Commands:")
    #print("  [timestamp_index] - Select a timestamp and show corresponding audio frames")
    print("  [timestamp_index] - Select a timestamp index and show timestamp")
    print("  st [start_timestamp] [end_timestamp] - Save segment from timestamp start_timestamp to end_timestamp")
    print("  st [start_timestamp] - Save from timestamp start_timestamp to end of file")
    print("  q - Exit")
    
    while True:
        try:
            user_input = input("\nEnter command: ").strip()
            
            if user_input.lower() == 'q':
                break
            
            parts = user_input.split()
            if len(parts) == 1:
                # Direct timestamp index input
                timestamp_index = int(parts[0])
                
                if not timestamps or timestamp_index < 0 or timestamp_index >= len(timestamps):
                    print(f"Error: Timestamp index must be between 0 and {len(timestamps)-1 if timestamps else 0}")
                    continue
                
                timestamp = timestamps[timestamp_index]
                start_audio_frame = timestamp_index * chunk_size
                
                # Calculate end audio frame for this timestamp
                if timestamp_index == total_timestamp_frames - 1:
                    # Last chunk might be incomplete
                    end_audio_frame = total_audio_frames - 1
                else:
                    end_audio_frame = (timestamp_index + 1) * chunk_size - 1
                
                start_time = start_audio_frame / sample_rate
                end_time = end_audio_frame / sample_rate
                
                print(f"Timestamp index: {timestamp_index}")
                print(f"Timestamp value: {timestamp}")
                #print(f"Corresponding audio frames: {start_audio_frame} to {end_audio_frame}")
                #print(f"Time range: {start_time:.6f} to {end_time:.6f} seconds")
                print(f"Time position: {start_time:.6f} seconds")
                #print(f"Duration: {end_time - start_time:.6f} seconds")
            
            elif len(parts) >= 2 and parts[0].lower() == 'st':
                # Save segment by timestamp indices - ensure saving at timestamp frame boundaries
                start_timestamp = int(parts[1])
                
                if len(parts) >= 3:
                    end_timestamp = int(parts[2])
                else:
                    end_timestamp = None
                
                if not timestamps:
                    print("Error: No timestamps available")
                    continue
                
                if start_timestamp < 0 or start_timestamp >= len(timestamps):
                    print(f"Error: Start timestamp index must be between 0 and {len(timestamps)-1}")
                    continue
                
                # Ensure saving at timestamp frame boundaries
                start_audio_frame = start_timestamp * chunk_size
                
                if end_timestamp is not None:
                    if end_timestamp < start_timestamp or end_timestamp >= len(timestamps):
                        print(f"Error: End timestamp index must be between {start_timestamp} and {len(timestamps)-1}")
                        continue
                    
                    # Ensure end position is also at timestamp frame boundary
                    #end_audio_frame = (end_timestamp + 1) * chunk_size - 1
                    end_audio_frame = (end_timestamp) * chunk_size - 1
                    
                    # Don't exceed file end
                    if end_audio_frame >= total_audio_frames:
                        end_audio_frame = total_audio_frames - 1
                        print(f"Note: Adjusted end frame to file end: {end_audio_frame}")
                else:
                    # Save to end of file
                    end_audio_frame = total_audio_frames - 1
                
                # Generate output filename using timestamp values
                base_name = os.path.splitext(os.path.basename(input_file))[0]
                start_ts_value = timestamps[start_timestamp]
                
                if end_timestamp is not None:
                    end_ts_value = timestamps[end_timestamp]
                    output_file = os.path.join(output_dir, f"{base_name}_timestamp_{start_ts_value}_to_{end_ts_value}.wav")
                else:
                    output_file = os.path.join(output_dir, f"{base_name}_timestamp_{start_ts_value}.wav")
                
                # Save segment
                if save_wav_segment(input_file, output_file, wav_info, start_audio_frame, end_audio_frame):
                    print(f"Segment saved to: {output_file}")
                    #print(f"Audio frames: {start_audio_frame} to {end_audio_frame}")
                    print(f"Timestamp indices: {start_timestamp} to {end_timestamp if end_timestamp is not None else 'end'}")
                else:
                    print("Failed to save segment")
            
            else:
                print("Invalid command. Use:")
                print("  [timestamp_index] - Select timestamp")
                print("  st [start_timestamp] [end_timestamp] - Save segment from timestamp start_timestamp to end_timestamp")
                print("  st [start_timestamp] - Save from timestamp start_timestamp to end of file")
                print("  q - Quit")
                
        except ValueError:
            print("Error: Please enter valid numbers")
        except KeyboardInterrupt:
            print("\nProgram interrupted by user")
            break

def main():
    # Get filename from command line arguments
    if len(sys.argv) < 2:
        print("Usage: python wav_frame_extractor.py <wav_file> [output_directory] [chunk_size]")
        print("Example: python wav_frame_extractor.py record.wav ./extracted_segments 128")
        return
    
    # File paths
    wav_filename = sys.argv[1]
    
    # Output directory
    if len(sys.argv) >= 3:
        output_dir = sys.argv[2]
    else:
        output_dir = "./extracted_segments"
    
    # Chunk size (default 128 for arecord)
    chunk_size = 128
    if len(sys.argv) >= 4:
        try:
            chunk_size = int(sys.argv[3])
        except ValueError:
            print("Error: Chunk size must be an integer")
            return
    
    # Create output directory if it doesn't exist
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        print(f"Created output directory: {output_dir}")
    
    ts_filename = wav_filename + ".ts"
    
    # 1. Parse WAV file
    print("Parsing WAV file...")
    wav_info = parse_wav_header(wav_filename, chunk_size)
    if not wav_info:
        return
    
    print("-" * 50)
    
    # 2. Load timestamp file
    print("Loading timestamp file...")
    timestamps = load_timestamps(ts_filename)
    
    print("-" * 50)
    
    # 3. Interactive frame selection and extraction
    select_frame_and_extract(wav_info, timestamps, wav_filename, output_dir)

if __name__ == "__main__":
    main()
